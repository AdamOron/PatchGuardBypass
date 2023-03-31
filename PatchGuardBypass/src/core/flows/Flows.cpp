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

	return (UINT64) DecodedDpc->DeferredRoutine == 0xfffff80323dd7330;
}
