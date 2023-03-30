#pragma once
#include <ntdef.h>

/* Forward declaration of KTIMER struct */
typedef struct _KTIMER
KTIMER, *PKTIMER;
/* Forward declaration of KDPC struct */
typedef struct _KDPC
KDPC, *PKDPC;

/**
Signature of a callback routine for each KTIMER encountered.
@param Timer is the KTIMER encountered.
@param DecodedDpc is the decoded DPC used by the Timer.
*/
typedef
VOID
TIMER_CALLBACK(
    PKTIMER Timer,
    PKDPC DecodedDpc
);
typedef TIMER_CALLBACK *PTIMER_CALLBACK;

/**
Iterates over all Timers in the system, invokes the given callbacks for each Timer.
*/
BOOLEAN
IterateSystemTimers(
    PTIMER_CALLBACK TimerCallback
);
