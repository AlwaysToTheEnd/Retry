#include "StaticObject.h"

std::list<StaticGameObjectController*> StaticGameObjectController::m_CurrObjects;
std::vector<StaticGameObjectController*> StaticGameObjectController::m_EndList;

void StaticGameObjectController::WorkALLEnd()
{
	for (auto& it : m_CurrObjects)
	{
		it->WorkClear();
	}

	m_CurrObjects.clear();
}

void StaticGameObjectController::WorkStart(bool otherWorksEnd)
{
	if (otherWorksEnd)
	{
		for (auto& it : m_CurrObjects)
		{
			if (it != this)
			{
				it->WorkClear();
			}
		}

		m_CurrObjects.clear();
	}
	else
	{
		for (auto iter = m_CurrObjects.begin(); iter != m_CurrObjects.end(); iter++)
		{
			if (*iter == this)
			{
				return;
			}
		}
	}

	m_CurrObjects.push_back(this);
}

void StaticGameObjectController::WorkEnd()
{
	WorkClear();

	m_EndList.push_back(this);
}

void StaticGameObjectController::StaticsUpdate()
{
	for (auto& it : m_EndList)
	{
		for (auto iter = m_CurrObjects.begin(); iter != m_CurrObjects.end(); iter++)
		{
			if (it == (*iter))
			{
				m_CurrObjects.erase(iter);
				break;
			}
		}
	}

	m_EndList.clear();

	for (auto iter = m_CurrObjects.begin(); iter != m_CurrObjects.end(); iter++)
	{
		(*iter)->Update();
	}
}
