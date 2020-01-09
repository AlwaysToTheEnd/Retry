#pragma once
#include "GameObject.h"


class TestObject :public GameObject
{
public:
	TestObject(CGHScene& scene, GameObject* parent, const char* typeName)
		: GameObject(scene, parent, typeName)
	{

	}
	virtual ~TestObject()=default;

private:
	virtual void Update(float delta) override;
	virtual void Init() override;

	void TestClickedFunc();
};