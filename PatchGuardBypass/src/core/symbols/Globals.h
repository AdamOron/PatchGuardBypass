#pragma once
#include <ntddk.h>

/* The Kernel KPRCB structure is undocumented, so we'll treat it as a void-pointer */
typedef PVOID PKPRCB;

namespace Globals
{
	namespace Variables
	{
		extern PINT64 KiWaitAlways;
		extern PINT64 KiWaitNever;
		extern PULONG KeNumberProcessors_0;
		extern PVOID *HalpClockTimer;
	};

	namespace Functions
	{
		/* Function pointer to KeGetPrcb, which is undocumented */
		extern PKPRCB (__fastcall *KeGetPrcb) (ULONG PrcbNumber);
		/* Invoked by the Context-Unaware Timers */
		extern PVOID CcBcbProfiler;
	};

	BOOLEAN
	Initialize(
		VOID
	);
};
