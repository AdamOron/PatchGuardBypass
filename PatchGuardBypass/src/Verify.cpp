#include "PatchGuard.h"
#include "timers/Timer.h"
#include "Flows.h"
#include "Log.h"
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
VOID
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
}

BOOLEAN
PG::Disable::Execute(
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
