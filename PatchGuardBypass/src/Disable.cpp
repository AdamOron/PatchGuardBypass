#pragma once
#include "PatchGuard.h"
#include "timers/Timer.h"
#include <ntddk.h>
#include "Log.h"

VOID
DisableTimers(
	PKTIMER Timer,
	PKDPC DecodedDpc
)
{
    if (!MmIsAddressValid(DecodedDpc))
        return;

    INT64 SpecialBit = (INT64) DecodedDpc->DeferredContext >> 47;

    if (SpecialBit != 0 && SpecialBit != -1)
    {
        RemoveEntryList(&Timer->TimerListEntry);

        Log("\nRemoved Context-Aware Timer/DPC: %p/%p\n", Timer, DecodedDpc);
    }

    if ((UINT64) DecodedDpc->DeferredRoutine == 0xfffff80323dd7330)
    {
        RemoveEntryList(&Timer->TimerListEntry);

        Log("\nRemoved Context-Unaware Timer/DPC: %p/%p\n", Timer, DecodedDpc);
    }
}

VOID
RegisterTimerCallbacks(
    TimerCallbackArray &DisableCallbacks
)
{
    DisableCallbacks.Append(NULL);
}

BOOLEAN
PG::Disable::Execute(
	VOID
)
{
    TimerCallbackArray disableCallbacks;
    RegisterTimerCallbacks(disableCallbacks);

    IterateSystemTimers(disableCallbacks);

    return TRUE;
}
