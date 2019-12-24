#include "StaticObject.h"

std::vector<StaticObject*> StaticObject::m_StaticObjects;

void StaticObject::StaticsInit()
{
	for (auto& it : m_StaticObjects)
	{
		it->Init();
	}
}

void StaticObject::StaticsUpdate()
{
	for (auto& it : m_StaticObjects)
	{
		it->Update();
	}
}
