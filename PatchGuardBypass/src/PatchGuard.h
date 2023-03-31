#pragma once
#include <ntdef.h>

namespace PG
{
	namespace Disable
	{
		BOOLEAN
		Execute(
			VOID
		);

		VOID
		Unload(
			VOID
		);
	};

	namespace Evade
	{
		BOOLEAN
		Execute(
			VOID
		);

		VOID
		Unload(
			VOID
		);
	};

	namespace Verify
	{
		BOOLEAN
		Execute(
			VOID
		);

		VOID
		Unload(
			VOID
		);
	};
};
