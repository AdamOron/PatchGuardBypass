#include "Flows.h"
#include <ntddk.h>
#include "../symbols/Globals.h"
#include "../symbols/Offsets.h"

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

	return DecodedDpc->DeferredRoutine == Globals::Functions::CcBcbProfiler;
}

ULONG
Flows::PrcbDpc::NextExecutionTime(
	VOID
)
{
	return *reinterpret_cast<PULONG>((PBYTE) *Globals::Variables::HalpClockTimer + Offsets::HalpClockTimer::NextExecutionTime);
}

PKDPC *
Flows::PrcbDpc::GetTargetDpc(
	VOID
)
{
	PKPRCB Prcb = Globals::Functions::KeGetPrcb(0);
	
	if (!Prcb)
		return NULL;

	return reinterpret_cast<PKDPC *>((PBYTE) Prcb + Offsets::Prcb::PatchGuardDpc);
}
