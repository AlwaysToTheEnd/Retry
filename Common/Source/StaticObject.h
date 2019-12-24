#pragma once
#include <vector>

class D3DApp;
class StaticObject
{
	friend class D3DApp;
public:
	StaticObject()
	{
		m_StaticObjects.push_back(this);
	}

	virtual ~StaticObject()
	{
		for (auto iter = m_StaticObjects.begin(); iter != m_StaticObjects.end(); iter++)
		{
			if (*iter == this)
			{
				m_StaticObjects.erase(iter);
				break;
			}
		}
	}
	
protected:
	virtual void Init() = 0;
	virtual void Update() = 0;

private:
	static void StaticsInit();
	static void StaticsUpdate();
	static std::vector<StaticObject*> m_StaticObjects;
};