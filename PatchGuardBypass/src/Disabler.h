#pragma once
#include "FlowHandler.h"

#define MAX_FLOW_INSTANCES 10

class Disabler : public FlowDisabler
{
private:
	static Disabler *s_Instance;

	FlowDisabler *m_Disablers[MAX_FLOW_INSTANCES];

public:
	static Disabler *GetInstance();

	Disabler();
	virtual bool Disable() override;
};

