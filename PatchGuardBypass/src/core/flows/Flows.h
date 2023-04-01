#pragma once
#include <ntdef.h>

/* Forward declaration of KTIMER struct */
typedef struct _KTIMER
KTIMER, *PKTIMER;
/* Forward declaration of KDPC struct */
typedef struct _KDPC
KDPC, *PKDPC;

/**
Contains sub-namespaces defining each execution flow for PatchGuard checks.
*/
namespace Flows
{
	/**
	PatchGuard issues a check through a Timer inserted to Prcb index 0.
	This Timer receives the PatchGuard Context struct as a parameter and uses it.
	*/
	namespace ContextAwareTimer
	{
		BOOLEAN
		IsTargetTimer(
			PKTIMER Timer,
			PKDPC DecodedDpc
		);
	};

	/**
	PatchGuard issues a check through a Timer inserted to Prcb index 0.
	This Timer does not receive the PatchGuard Context struct as a parameter.
	It is more static & less complex than the context aware Timer.
	*/
	namespace ContextUnawareTimer
	{
		BOOLEAN
		IsTargetTimer(
			PKTIMER Timer,
			PKDPC DecodedDpc
		);
	};

	/**
	PatchGuad issues a check through a DPC saved to Prcb index 0.
	This DPC is inserted to the HalReserved[7] field from FsRtlMdlReadCompleteDevEx.
	It is executed every 2 minutes from HalpMcaQueueDpc.
	*/
	namespace PrcbDpc
	{
		/**
		@return The next execution time of the DPC stored in the Prcb.
		This value is returned as an interrupt timestamp.
		*/
		ULONG
		NextExecutionTime(
			VOID
		);

		/**
		@return Pointer to the target DPC stored in the Prcb.
		This returns a pointer to allow easily overwriting the DPC.
		*/
		PKDPC *
		GetTargetDpc(
			VOID
		);
	};
};
