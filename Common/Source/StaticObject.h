#pragma once
#include <vector>

class D3DApp;
class StaticGameObjectController
{
	friend class D3DApp;
public:
	StaticGameObjectController()
	{
	}

	virtual ~StaticGameObjectController()
	{
		if (this == m_CurrObject)
		{
			m_CurrObject = nullptr;
		}
	}
	
protected:
	virtual void Init() = 0;
	virtual void Update() = 0;
	virtual void WorkClear();
	void WorkStart(); // workStart should be excuted on CGH::ExcuteFuncOfClickedObject.

private:
	static void StaticsUpdate();

private:
	static StaticGameObjectController*	m_CurrObject;
};