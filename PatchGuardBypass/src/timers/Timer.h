#pragma once
#include "../utils/FixedArray.h"
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

/* Typedef for a fixed array of TIMER_CALLBACK routines */
#define MAX_TIMER_CALLBACKS 10
typedef FixedArray<PTIMER_CALLBACK, MAX_TIMER_CALLBACKS> TimerCallbackArray;

/**
Iterates over all Timers in the system, invokes the given callbacks for each Timer.
@param TimerCallbacks is the fixed array of callbacks to be invoked.
*/
BOOLEAN
IterateSystemTimers(
    TimerCallbackArray &TimerCallbacks
);
