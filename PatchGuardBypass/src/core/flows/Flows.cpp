#include "Flows.h"
#include <ntddk.h>

BOOLEAN
Flows::ContextAwareTimer::IsTargetTimer(
	PKTIMER Timer,
	PKDPC DecodedDpc
)
{
	UNREFERENCED_PARAMETER(Timer);

    if (!MmIsAddressValid(DecodedDpc))
        return FALSE;

    INT64 SpecialBit = (INT64) DecodedDpc->DeferredContext >> 47;
    return SpecialBit != 0 && SpecialBit != -1;
}

BOOLEAN
Flows::ContextUnawareTimer::IsTargetTimer(
	PKTIMER Timer,
	PKDPC DecodedDpc
)
{
	UNREFERENCED_PARAMETER(Timer);

	if (!MmIsAddressValid(DecodedDpc))
		return FALSE;

	/*
	This magic number is the address of CcBcbProfiler on a snapshot I'm using.
	I'll update this in the future when I set up a proper Symbol system.
	*/
	return (UINT64) DecodedDpc->DeferredRoutine == 0xfffff80323dd7330;
}
