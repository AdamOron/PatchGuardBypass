#include "Disabler.h"
#include <ntdef.h>

Disabler::Disabler() : 
	m_Disablers()
{
}

Disabler *Disabler::GetInstance()
{
	//if (!s_Instance)
	//	s_Instance = new Disabler();
	
	return s_Instance;
}

bool Disabler::Disable() 
{
	for (FlowDisabler *disabler : m_Disablers)
		if (!disabler->Disable())
			return false;

	return true;
}

Disabler *Disabler::s_Instance = NULL;

