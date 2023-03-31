#include "PatchGuard.h"
#include "timers/Timer.h"
#include "Flows.h"
#include "Log.h"
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

typedef struct _EVADE_CONTEXT
{
#define MAX_PG_TIMERS 10
    PKTIMER Timers[MAX_PG_TIMERS];
    UINT32 TimerCount;
    ULONGLONG CurrentTimer;

    KDPC StartEvasionDpc;
    KTIMER StartEvasionTimer;

    KDPC StopEvasionDpc;
    KTIMER StopEvasionTimer;
} EVADE_CONTEXT, PEVADE_CONTEXT;

EVADE_CONTEXT g_EvadeContext = { 0 };

inline
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
VOID
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
}

BOOLEAN
UpdateTimers(
    VOID
)
{
    /* Reset all previously found Timers */
    g_EvadeContext.TimerCount = 0;
    /* Find all PG related Timers */
    IterateSystemTimers(&FindTimer);

    if (TimerOverflow())
        return FALSE;

    return TRUE;
}

ULONGLONG
GetEarliestTimer(
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

        if (currentTime < earliestTime)
            earliestTime = currentTime;
    }

    return earliestTime;
}

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

BOOLEAN
SetStartEvasionTimer(
    BOOLEAN RequiresUpdate
)
{
    /* If we require an update and the update failed */
    if (RequiresUpdate && !UpdateTimers())
        return FALSE;

    g_EvadeContext.CurrentTimer = GetEarliestTimer();

    LARGE_INTEGER startEvasionTime;
    startEvasionTime.QuadPart = g_EvadeContext.CurrentTimer /* - 1000 something, so it's called earlier */;

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
    PVOID SystemArgument1,
    PVOID SystemArgument2
)
{
    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(DeferredContext);
    UNREFERENCED_PARAMETER(SystemArgument1);
    UNREFERENCED_PARAMETER(SystemArgument2);

    UINT32 prevTimerCount = g_EvadeContext.TimerCount;

    if (!UpdateTimers())
        return;
    
    /* Ensure that previus Timer was actually removed */
    if (g_EvadeContext.CurrentTimer == GetEarliestTimer())
    {
        /* Do other stuff, sleep longer maybe idk */
        return;
    }

    /* Check that a new replacement Timer was inserted, i.e. execution finished */
    if (prevTimerCount == g_EvadeContext.TimerCount)
    /* && CurrentTime < GetEarliestTimer(), make sure next Timer isn't pending or smth*/
    {
        KeCancelTimer(&g_EvadeContext.StopEvasionTimer);
        StopEvasion();
    }
}

BOOLEAN
SetStopEvasionTimer(
    VOID
)
{
    LARGE_INTEGER dueTime;
    dueTime.QuadPart = /* CurrentInterruptTime + */ MS_TO_HNS(STOP_INITIAL_COOLOWN);

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
