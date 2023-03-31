#include "../PatchGuard.h"
#include "../timers/Timer.h"
#include "../flows/Flows.h"
#include "../../utils/Log.h"
#include <ntddk.h>

/**
For each Timer encountered, checks if it's PG related and removes it if so.
Compatible with the TIMER_CALLBACK signature, passed to IterateSystemTimers.
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

        Log("\nRemoved Context-Aware Timer/DPC: %p/%p\n", Timer, DecodedDpc);
    }

    if (Flows::ContextUnawareTimer::IsTargetTimer(Timer, DecodedDpc))
    {
        KeCancelTimer(Timer);

        Log("\nRemoved Context-Unaware Timer/DPC: %p/%p\n", Timer, DecodedDpc);
    }

    return ContinueTimerSearch;
}

BOOLEAN
PG::Disable::Execute(
	VOID
)
{
    /* Disable all PG related Timers */
    SearchSystemTimers(&RemoveTimer);

    /* Flows::PrcbDpc:: */

    return TRUE;
}

VOID
PG::Disable::Unload(
    VOID
)
{
    /* Nothing to do here */
}
