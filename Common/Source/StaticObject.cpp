#include "StaticObject.h"

StaticGameObjectController* StaticGameObjectController::m_CurrObject = nullptr;

void StaticGameObjectController::WorkClear()
{
	m_CurrObject = nullptr;
}

void StaticGameObjectController::WorkStart()
{
	if (m_CurrObject)
	{
		m_CurrObject->WorkClear();
	}

	m_CurrObject = this;
}

void StaticGameObjectController::StaticsUpdate()
{
	if (m_CurrObject)
	{
		m_CurrObject->Update();
	}
}
