#include "StaticObject.h"
#include <assert.h>

std::list<StaticGameObjectController*> StaticGameObjectController::m_CurrObjects;
std::vector<StaticGameObjectController*> StaticGameObjectController::m_EndList;
std::vector<StaticGameObjectController*> StaticGameObjectController::m_Residents;

void StaticGameObjectController::WorkALLEnd()
{
	for (auto& it : m_CurrObjects)
	{
		it->WorkClear();
	}

	for (auto& it : m_Residents)
	{
		it->WorkClear();
	}

	m_CurrObjects.clear();
}

void StaticGameObjectController::WorkAllClear()
{
	for (auto& it : m_CurrObjects)
	{
		it->WorkClear();
	}

	for (auto& it : m_Residents)
	{
		it->WorkClear();
	}
}

void StaticGameObjectController::WorkStart(bool otherWorksEnd)
{
	assert(m_isResident == false);

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
	assert(m_isResident == false);
	m_EndList.push_back(this);
}

void StaticGameObjectController::StaticsUpdate(unsigned long long delta)
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

	for (auto& it : m_Residents)
	{
		it->Update(delta);
	}

	for (auto iter = m_CurrObjects.begin(); iter != m_CurrObjects.end(); iter++)
	{
		(*iter)->Update(delta);
	}
}
