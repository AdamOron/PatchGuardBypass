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
	TODO: Get this from Symbol manager, when there is one.
	*/
	return (UINT64) DecodedDpc->DeferredRoutine == 0xfffff80323dd7330;
}

ULONG
Flows::PrcbDpc::NextExecutionTime(
	VOID
)
{
	/* TODO: Get this from Symbol manager, when there is one */
	UINT32 *HalpClockTimer = (UINT32 *) 0x1;
	/* Offset 0x3C from HalpClockTimer */
	return HalpClockTimer[0xF];
}

PKDPC *
Flows::PrcbDpc::GetTargetDpc(
	VOID
)
{
	/* TODO: Get this from Symbol manager, when there is one */
	PKDPC *Prcb = (PKDPC *) 0x1;
	/* Offset 0x80 from first Prcb */
	return &Prcb[0x10];
}
