#pragma once

class FlowHandler
{
protected:
	FlowHandler()
	{
	}
};

/**
* Handler used for completely disabling a PatchGuard execution flow.
* This handler (ideally) only executes once, and simply disables a certain PatchGuard
* execution flow, thereby preventing that certain test.
*/
class FlowDisabler : public FlowHandler
{
public:
	virtual bool Disable() = 0;
};

/**
* Handler used for completely evading a PatchGuard execution flow.
* This handler does not disable PatchGuard, nor does it modify any kernel value at all;
* it only alerts the user prior to a PatchGuard test.
*/
class FlowEvader : public FlowHandler
{
public:
	/* Virtual Functions for end-user handling before/after a test is triggered */
	virtual void BeforeTest() = 0;
	virtual void AfterTest() = 0;
};

class FlowVerifier : public FlowHandler
{
public:
	virtual bool Verify() = 0;
};

