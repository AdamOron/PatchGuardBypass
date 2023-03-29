#pragma once
#include <ntifs.h>
#include <ntddk.h>
#include <windef.h>

namespace PG
{
	namespace Disabler
	{
		BOOL Execute();
	};

	namespace Evader 
	{
#define MAX_EVADER_PCALLBACKS 10 

		enum ALERT_MODE
		{
			BeforeTest,
			AfterTest
		};

		typedef
		VOID 
		CALLBACK_FUNCTION (
			ALERT_MODE AlertMode
		);
		
		typedef CALLBACK_FUNCTION *PCALLBACK_FUNCTION;

		using FixedCallbacksArray = FixedArray<PCALLBACK_FUNCTION, MAX_EVADER_PCALLBACKS>;

		BOOL
		RegisterCallback(
			PCALLBACK_FUNCTION CallbackFunction
		);

		BOOL Execute();
	};

	namespace Verifier 
	{
		BOOL Execute();
	};
};

BOOL PG::Disabler::Execute()
{
	return FALSE;
}

BOOL PG::Evader::Execute()
{
	return FALSE;
}

BOOL
PG::Evader::RegisterCallback(
	PCALLBACK_FUNCTION CallbackFunction
)
{
	return EvaderCallbacks += CallbackFunction;
}

BOOL PG::Verifier::Execute()
{
	return FALSE;
}

