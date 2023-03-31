#include "../PatchGuard.h"
#include "../timers/Timer.h"
#include "../flows/Flows.h"
#include "../../utils/Log.h"
#include <ntddk.h>

typedef struct _VERIFY_CONTEXT
{
    BOOLEAN bContextAwareTimer;
    BOOLEAN bContextUnawareTimer;
    BOOLEAN bPrcbDpc;
} VERIFY_CONTEXT, *PVERIFY_CONTEXT;

VERIFY_CONTEXT g_VerifyContext = { FALSE };

/**
For each Timer encountered, checks if it's PG related and removes it if so.
Compatible with the TIMER_CALLBACK signature, passed to IterateSystemTimers.
*/
TIMER_SEARCH_STATUS
VerifyTimer(
    PKTIMER Timer,
    PKDPC DecodedDpc
)
{
    if (Flows::ContextAwareTimer::IsTargetTimer(Timer, DecodedDpc))
    {
        g_VerifyContext.bContextAwareTimer = TRUE;
    }

    if (Flows::ContextUnawareTimer::IsTargetTimer(Timer, DecodedDpc))
    {
        g_VerifyContext.bContextUnawareTimer = TRUE;
    }

    return ContinueTimerSearch;
}

BOOLEAN
PG::Verify::Execute(
    VOID
)
{
    /* Verify all PG related Timers exist */
    IterateSystemTimers(&VerifyTimer);

    return
        g_VerifyContext.bContextAwareTimer &&
        g_VerifyContext.bContextUnawareTimer &&
        g_VerifyContext.bPrcbDpc;
}

VOID
PG::Verify::Unload(
    VOID
)
{
    /* Nothing to do here */
}
