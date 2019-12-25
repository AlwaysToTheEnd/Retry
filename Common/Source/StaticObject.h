#pragma once
#include <list>
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
	
	}
	
protected:
	virtual void Init() = 0;
	virtual void Update() = 0;
	virtual void WorkClear() = 0;
	static void WorkALLEnd();
	void WorkStart(bool otherWorksEnd=false); // workStart should be excuted on CGH::ExcuteFuncOfClickedObject.
	void WorkEnd();

private:
	static void StaticsUpdate();

private:
	static std::list<StaticGameObjectController*> m_CurrObjects;
	static std::vector<StaticGameObjectController*> m_EndList;
};