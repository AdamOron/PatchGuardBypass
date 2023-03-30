#include "PatchGuard.h"
#include "timers/Timer.h"
#include "Flows.h"
#include "Log.h"
#include <ntddk.h>

/**
For each Timer encountered, checks if it's PG related and removes it if so.
Compatible with the TIMER_CALLBACK signature, passed to IterateSystemTimers.
*/
VOID
VerifyTimer(
	PKTIMER Timer,
	PKDPC DecodedDpc
)
{
    if (Flows::ContextAwareTimer::IsTargetTimer(Timer, DecodedDpc))
    {
        RemoveEntryList(&Timer->TimerListEntry);

        Log("\nRemoved Context-Aware Timer/DPC: %p/%p\n", Timer, DecodedDpc);
    }

    if (Flows::ContextUnawareTimer::IsTargetTimer(Timer, DecodedDpc))
    {
        RemoveEntryList(&Timer->TimerListEntry);

        Log("\nRemoved Context-Unaware Timer/DPC: %p/%p\n", Timer, DecodedDpc);
    }
}

BOOLEAN
PG::Disable::Execute(
	VOID
)
{
    /* Disable all PG related Timers */
    IterateSystemTimers(&VerifyTimer);

    /* Flows::PrcbDpc:: */

    return TRUE;
}
