#pragma once
#include "Timer.h"
#include <ntifs.h>
#include <ntddk.h>
#include "../Log.h"

/**
* Kernel structure defining an entry within a KTIMER_TABLE.
* Each entry contains a linked-list of KTIMERs.
*/
typedef struct _KTIMER_TABLE_ENTRY
{
    /* Locked used to synchronize access to the entry */
    unsigned __int64 Lock;
    /* Head of a linked-list of KTIMERs */
    LIST_ENTRY Entry;
    /* Earliest expiration time within this entry's Timers (used in insertion process, iirc) */
    ULARGE_INTEGER Time;
} KTIMER_TABLE_ENTRY, *PKTIMER_TABLE_ENTRY;

/* Size of the TimerExpiry array */
#define TIMER_EXPIRY_SIZE 64
/* Size of the TimerEntries array */
#define TIMER_ENTRIES_SIZE 256

typedef struct _KTIMER_TABLE
{
    /* An array of KTIMER pointers, representing all expired Timers that need processing */
    PKTIMER TimerExpiry[TIMER_EXPIRY_SIZE];
    /*
    A 2-dimensional array of KTIMER_TABLE_ENTRIES, containing all user-mode & kernel-mode
    (I think?) non-expired Timers. TimerEntries[0] is for kernel-mode, TimerEntries[1] for user-mode.
    */
    KTIMER_TABLE_ENTRY TimerEntries[2][TIMER_ENTRIES_SIZE];
    /* TimerState structure (size=0x18), we do not care for this */
    char TableState[0x18]; 
} KTIMER_TABLE, *PKTIMER_TABLE;

/* The Kernel KPRCB structure is undocumented, so we'll treat it as a void-pointer */
typedef PVOID PKPRCB;

/* The known offset to the TimerTable field within a KPRCB */
#define TIMER_TABLE_OFFSET 0x3940

/* Retrieves the TimerTable field of a KPRCB */
PKTIMER_TABLE
GetTimerTable(
    PKPRCB Prcb
)
{
    return (PKTIMER_TABLE) ((char *) Prcb + TIMER_TABLE_OFFSET);
}

/*
The following values are just magic numbers that are valid for a specific snapshot I have.
Once a proper Symbol Manager is introduced, we'll be able to easily get the actual values of these
*/

/* Keys used for decoding/encoding DPCs within KTIMERs */
INT64 KiWaitAlways = 0x007ec1055bfd1db0;
INT64 KiWaitNever = 0xe8e7003f609ce7e4;
/* Number of processor on the system. Also indicates number of PRCBs */
ULONG KeNumberProcessors_0 = 4;

/*
Returns the decoded pointer to the DPC stored in a KTIMER structre.
These DPC pointers are encoded during insertion of the Timers.
*/
#define DECODE_TIMER_DPC(Timer) \
    (PKDPC) (KiWaitAlways ^ _byteswap_uint64( \
        (UINT64) Timer ^ _rotl64( \
            (INT64) Timer->Dpc ^ KiWaitNever, \
            (UCHAR) KiWaitNever \
        )))

/** 
Iterates over all entries of a linked-list of KTIMERs.
For each KTIMER, the given callback function is invoked.
@param TimerListHead is the head entry of the list, from a KTIMER_TABLE_ENTRY structure.
@param TimerCallbacks is a fixed array of TIMER_CALLBACK routines, invoked for each KTIMER.
@return search instructions for the caller.
*/
TIMER_SEARCH_STATUS
IterateTimerList(
    PLIST_ENTRY TimerListHead,
    PTIMER_CALLBACK TimerCallback
)
{
    PLIST_ENTRY pListEntry = TimerListHead->Flink;

    /* As long as the current entry is valid and we haven't reached the end */
    while (pListEntry && pListEntry != TimerListHead)
    {
        /* Get the KTIMER that contains the current list entry */
        PKTIMER pTimer = CONTAINING_RECORD(
            pListEntry,
            KTIMER,
            TimerListEntry
        );

        /* Get the decoded DPC */
        PKDPC pDpc = DECODE_TIMER_DPC(pTimer);

        /* Invoke registered callback */
        if (TimerCallback(pTimer, pDpc) == StopTimerSearch)
            return StopTimerSearch;

        /* Advance to the next Timer entry */
        pListEntry = pListEntry->Flink;
    }

    return ContinueTimerSearch;
}

/**
Iterates all pending KTIMERs in the given KTIMER_TABLE.
@param TimerTable is the KTIMER_TABLE we want to iterate.
@param TimerCallbacks is a fixed array of TIMER_CALLBACK routines, invoked for each KTIMER.
*/
VOID
IterateTimerTable(
    PKTIMER_TABLE TimerTable,
    PTIMER_CALLBACK TimerCallback
)
{
    /* Get the array of kernel-mode KTIMER_TABLE_ENTRYs */
    PKTIMER_TABLE_ENTRY pKernelEntries = TimerTable->TimerEntries[KernelMode];
    /* Get the array of user-mode KTIMER_TABLE_ENTRYs */
    PKTIMER_TABLE_ENTRY pUserEntries = TimerTable->TimerEntries[UserMode];

    /* Iterate over all KTIMER_TABLE_ENTRYs in both arrays */
    for (USHORT i = 0; i < TIMER_ENTRIES_SIZE; i++)
    {
        /* Iterate linked-list of KTIMERs within each KTIMER_TABLE_ENTRY */

        if (IterateTimerList(&pKernelEntries[i].Entry, TimerCallback) == StopTimerSearch)
            break;

        if (IterateTimerList(&pUserEntries[i].Entry, TimerCallback) == StopTimerSearch)
            break;
    }
}

/**
Iterates over all Timers in the system, invokes the given callbacks for each Timer.
@param TimerCallbacks is the fixed array of callbacks to be invoked.
*/
BOOLEAN
IterateSystemTimers(
    PTIMER_CALLBACK TimerCallback
)
{
    /* For every processor in the system */
    for (ULONG i = 0; i < KeNumberProcessors_0; i++)
    {
        /* Get the matching KPRCB struct */
        PKPRCB pPrcb = KeGetPcr()->CurrentPrcb;

        if (!pPrcb)
            return FALSE;

        /* Get the KPRCB's TimerTable, then iterate its Timers */
        IterateTimerTable(GetTimerTable(pPrcb), TimerCallback);
    }

    return TRUE;
}
