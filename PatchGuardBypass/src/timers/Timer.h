#pragma once
#include <ntdef.h>

/* Forward declaration of KTIMER struct */
typedef struct _KTIMER
KTIMER, *PKTIMER;
/* Forward declaration of KDPC struct */
typedef struct _KDPC
KDPC, *PKDPC;

/**
Enum returned by TIMER_CALLBACK to indicate how the search should continue.
*/
typedef enum _TIMER_SEARCH_STATUS
{
    StopTimerSearch,
    ContinueTimerSearch,
} TIMER_SEARCH_STATUS, *PTIMER_SEARCH_STATUS;

/**
Signature of a callback routine for each KTIMER encountered.
@param Timer is the KTIMER encountered.
@param DecodedDpc is the decoded DPC used by the Timer.
@return TRUE if execution should continue, FASLE if it should stop.
*/
typedef
TIMER_SEARCH_STATUS
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
