#pragma once
#include <windef.h>

namespace Offsets
{
	/* Offsets to fields within a Prcb (struct _KPRCB) */
	namespace Prcb
	{
		/* Offset to TimerTable field */
		extern UINT32 TimerTable;
		extern UINT32 PatchGuardDpc;
	};

	namespace HalpClockTimer
	{
		extern UINT32 NextExecutionTime;
	};
};
