#include "../PatchGuard.h"
#include "../timers/Timer.h"
#include "../flows/Flows.h"
#include "../../utils/Log/Log.h"
#include <ntddk.h>

/**
For each Timer encountered, checks if it's PG related and removes it if so.
Compatible with the TIMER_CALLBACK signature, passed to SearchSystemTimers.
*/
TIMER_SEARCH_STATUS
RemoveTimer(
	PKTIMER Timer,
	PKDPC DecodedDpc
)
{
    if (Flows::ContextAwareTimer::IsTargetTimer(Timer, DecodedDpc))
    {
        KeCancelTimer(Timer);

        Log("Removed Context-Aware Timer/DPC: %p/%p\n", Timer, DecodedDpc);
    }

    if (Flows::ContextUnawareTimer::IsTargetTimer(Timer, DecodedDpc))
    {
        KeCancelTimer(Timer);

        Log("Removed Context-Unaware Timer/DPC: %p/%p\n", Timer, DecodedDpc);
    }

    return ContinueTimerSearch;
}

/**
Removes the DPC stored in the Prcb structure, i.e. NULLs it.
According to HalpMcaQueueDpc, a NULL DPC does not result in an error.
*/
BOOLEAN
RemovePrcbDpc(
    VOID
)
{
    PKDPC *PrcbDpc = Flows::PrcbDpc::GetTargetDpc();
    
    if (!PrcbDpc)
        return FALSE;

    if (!*PrcbDpc)
        return TRUE;

    Log("Removed Prcb DPC: %p\n", *PrcbDpc);

    *PrcbDpc = NULL;

    return TRUE;
}

BOOLEAN
PG::Disable::Execute(
	VOID
)
{
    /* Disable all PG related Timers (prevents ContextAware/Unaware flows) */
    SearchSystemTimers(&RemoveTimer);

    /* Remove DPC from Prcb (prevents Prcb DPC flow) */
    if (!RemovePrcbDpc())
        return FALSE;

    return TRUE;
}

VOID
PG::Disable::Unload(
    VOID
)
{
    /* Nothing to do here */
}
