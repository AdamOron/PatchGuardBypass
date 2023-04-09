#include "../PatchGuard.h"
#include "../timers/Timer.h"
#include "../flows/Flows.h"
#include "../../utils/Log/Log.h"
#include <ntddk.h>

/**
Macro to convert milliseconds to hecto-nanoseconds, i.e. 100-nanoseconds.
This is the term for Windows interrupt time units.
*/
#define MS_TO_HNS(x) (x * 10000)

/* Initial StopTimer expiry cooldown (DueTime parameter) */
#define STOP_INITIAL_COOLOWN 1000
/* StopTimer cooldown after initial expiry (Period parameter) */
#define STOP_COOLOWN 500

/**
Global context for the evasion feature.
*/
typedef struct _EVADE_CONTEXT
{
    /*
    Array of PG-related KTIMERs. The array can contain up to 10 entries.
    I'm using an array for this because I'm not 100% confident that only 2 timers
    can exist at a time. If that's the case, this design is redundant and will change.
    */
#define MAX_PG_TIMERS 10
    PKTIMER Timers[MAX_PG_TIMERS];
    UINT32 TimerCount;
    /* The expiration time of the Timer we're currently avoiding */
    ULONGLONG LastAvoidedExpiration;
    /* Define a Timer for starting the evasion process */
    KDPC StartEvasionDpc;
    KTIMER StartEvasionTimer;
    /* Define a Timer for stopping the evasion process */
    KDPC StopEvasionDpc;
    KTIMER StopEvasionTimer;
} EVADE_CONTEXT, PEVADE_CONTEXT;

EVADE_CONTEXT g_EvadeContext = { 0 };

VOID
InsertTimerToContext(
    PKTIMER Timer
)
{
    if (g_EvadeContext.TimerCount >= MAX_PG_TIMERS)
    {
        g_EvadeContext.TimerCount++;
        return;
    }

    g_EvadeContext.Timers[g_EvadeContext.TimerCount++] = Timer;
}

FORCEINLINE
BOOLEAN
TimerOverflow(
    VOID
)
{
    return g_EvadeContext.TimerCount > MAX_PG_TIMERS;
}

/**
For each Timer encountered, checks if it's PG related and saves it if so.
Compatible with the TIMER_CALLBACK signature, passed to IterateSystemTimers.
*/
TIMER_SEARCH_STATUS
FindTimer(
    PKTIMER Timer,
    PKDPC DecodedDpc
)
{
    if (Flows::ContextAwareTimer::IsTargetTimer(Timer, DecodedDpc))
    {
        InsertTimerToContext(Timer);
    }

    if (Flows::ContextUnawareTimer::IsTargetTimer(Timer, DecodedDpc))
    {
        InsertTimerToContext(Timer);
    }

    return ContinueTimerSearch;
}

BOOLEAN
UpdateTimers(
    VOID
)
{
    /* Reset all previously found Timers */
    g_EvadeContext.TimerCount = 0;
    /* Find all PG related Timers */
    SearchSystemTimers(&FindTimer);

    if (TimerOverflow())
        return FALSE;

    return TRUE;
}

ULONGLONG
EarliestTimerExpiration(
    VOID
)
{
    ULONGLONG earliestTime = MAXLONGLONG;

    for (UINT32 i = 0; i < g_EvadeContext.TimerCount; i++)
    {
        PKTIMER currentTimer = g_EvadeContext.Timers[i];

        if (!currentTimer)
            continue;

        ULONGLONG currentTime = currentTimer->DueTime.QuadPart;

        if (currentTimer->Period)
        {
            /* BP if Timer is periodical. Need to figure out how to calculate the DueTime */
            DbgBreakPoint();
        }

        if (currentTime < earliestTime)
            earliestTime = currentTime;
    }

    return earliestTime;
}

/* Start hiding a bit earlier than the Timers expire */
#define EVASION_TIMER_UNDERSHOOT 500

BOOLEAN
SetStopEvasionTimer(
    VOID
);

VOID
StartEvasion(
    PKDPC Dpc,
    PVOID DeferredContext,
    PVOID SystemArgument1,
    PVOID SystemArgument2
)
{
    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(DeferredContext);
    UNREFERENCED_PARAMETER(SystemArgument1);
    UNREFERENCED_PARAMETER(SystemArgument2);

    // DisableAllPatches();

    SetStopEvasionTimer();
}

#define AVOIDING_TIMER ((PVOID) 0x1)
#define AVOIDING_DPC ((PVOID) 0x0)

BOOLEAN
SetStartEvasionTimer(
    BOOLEAN RequiresUpdate
)
{
    /* If we require an update and the update failed */
    if (RequiresUpdate && !UpdateTimers())
        return FALSE;

    ULONGLONG timerExpiration = EarliestTimerExpiration();
    ULONG dpcExecution = Flows::PrcbDpc::NextExecutionTime();

    if (timerExpiration < dpcExecution)
    {
        g_EvadeContext.LastAvoidedExpiration = timerExpiration;
        /* Indicate to the DPC that we're avoiding a Timer, and not the Prcb DPC */
        g_EvadeContext.StartEvasionDpc.SystemArgument1 = AVOIDING_TIMER;
    }
    else
    {
        g_EvadeContext.LastAvoidedExpiration = dpcExecution;
        /* Indicate to the DPC that we're avoiding the Prcb DPC, and not a Timer */
        g_EvadeContext.StartEvasionDpc.SystemArgument1 = AVOIDING_DPC;
    }

    LARGE_INTEGER startEvasionTime;
    startEvasionTime.QuadPart = g_EvadeContext.LastAvoidedExpiration - EVASION_TIMER_UNDERSHOOT;

    if (KeSetTimer(
        &g_EvadeContext.StartEvasionTimer,
        startEvasionTime,
        &g_EvadeContext.StartEvasionDpc
    ))
        return FALSE;

    return TRUE;
}

VOID
StopEvasion(
    VOID
)
{
    // EnableAllPatches();

    SetStartEvasionTimer(FALSE);
}

VOID
TryStopEvasion(
    PKDPC Dpc,
    PVOID DeferredContext,
    PVOID IsAvoidingTimer,
    PVOID pvAttemptCount
)
{
    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(DeferredContext);

    /* Convert from PVOID to UINT64 */
    UINT64 attemptCount = (UINT64) pvAttemptCount;

    if (attemptCount >= 5)
    {
        /*
        BP if we've attempted & failed more than 5 times.
        TODO: Better error handling (log error to user, exit program?)
        */
        DbgBreakPoint();
        goto Exit;
    }

    /* Increment attempt count. Why am I using global variables smh */
    g_EvadeContext.StopEvasionDpc.SystemArgument2 = (PVOID) (attemptCount + 1);

    /* Timer count before evasion started */
    UINT32 prevTimerCount = g_EvadeContext.TimerCount;

    /* Search again for all PG related Timers */
    if (!UpdateTimers())
        return;
    
    /* If we're avoiding a Timer, ensure it was actually removed (it's no longer the earliest) */
    if (IsAvoidingTimer == AVOIDING_TIMER &&
        g_EvadeContext.LastAvoidedExpiration == EarliestTimerExpiration())
    {
        /* TODO: Maybe do other stuff, we can sleep longer or something */
        return;
    }

    /* Check that a new replacement Timer was inserted, i.e. execution finished */
    if (prevTimerCount == g_EvadeContext.TimerCount)
    /* && CurrentTime < GetEarliestTimer(), make sure next Timer isn't already executing? not sure if necessary */
    {
        StopEvasion();

        Exit:
        /* If we finished avoiding, cancel the Timer */
        KeCancelTimer(&g_EvadeContext.StopEvasionTimer);
    }
}

BOOLEAN
SetStopEvasionTimer(
    VOID
)
{
    LARGE_INTEGER dueTime;
    dueTime.QuadPart = /* CurrentTime + */ MS_TO_HNS(STOP_INITIAL_COOLOWN);

    /* Notify StopEvasionDpc whether we're avoiding a Timer or the Prcb DPC */
    g_EvadeContext.StopEvasionDpc.SystemArgument1 = g_EvadeContext.StartEvasionDpc.SystemArgument1;
    /* Initialize attempt count to zero */
    g_EvadeContext.StartEvasionDpc.SystemArgument2 = 0;

    if (KeSetTimerEx(
        &g_EvadeContext.StopEvasionTimer,
        dueTime,
        STOP_COOLOWN,
        &g_EvadeContext.StopEvasionDpc
    ))
        return FALSE;

    return TRUE;
}

BOOLEAN
PrepareContext(
    VOID
)
{
    KeInitializeDpc(
        &g_EvadeContext.StartEvasionDpc,
        StartEvasion,
        NULL
    );

    KeInitializeTimerEx(
        &g_EvadeContext.StartEvasionTimer,
        NotificationTimer
    );

    KeInitializeDpc(
        &g_EvadeContext.StopEvasionDpc,
        TryStopEvasion,
        NULL
    );

    KeInitializeTimerEx(
        &g_EvadeContext.StopEvasionTimer,
        NotificationTimer
    );

    return TRUE;
}

BOOLEAN
PG::Evade::Execute(
    VOID
)
{
    if (!PrepareContext())
        return FALSE;

    if (!SetStartEvasionTimer(TRUE))
        return FALSE;

    return TRUE;
}

VOID
PG::Evade::Unload(
    VOID
)
{
    /* Cancel current Timers */
    KeCancelTimer(&g_EvadeContext.StartEvasionTimer);
    KeCancelTimer(&g_EvadeContext.StopEvasionTimer);
}
