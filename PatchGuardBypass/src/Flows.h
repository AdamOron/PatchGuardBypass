#pragma once
#include <ntdef.h>

/* Forward declaration of KTIMER struct */
typedef struct _KTIMER
KTIMER, *PKTIMER;
/* Forward declaration of KDPC struct */
typedef struct _KDPC
KDPC, *PKDPC;

namespace Flows
{
	namespace ContextAwareTimer
	{
		typedef struct _STATE
		STATE, *PSTATE;

		PSTATE
		GetState(
			VOID
		);

		VOID
		GetTargetTimer(
			PKTIMER Timer,
			PKDPC DecodedDpc
		);
	};

	namespace ContextUnawareTimer
	{
		typedef struct _STATE
		STATE, *PSTATE;

		PSTATE
		GetState(
			VOID
		);

		VOID
		GetTargetTimer(
			PKTIMER Timer,
			PKDPC DecodedDpc
		);
	};

	namespace PrcbDpc
	{
		typedef struct _STATE
		STATE, *PSTATE;

		PSTATE
		GetState(
			VOID
		);
	};
};
