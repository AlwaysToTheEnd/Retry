#pragma once
#include "GameObject.h"


class TestObject :public GameObject
{
public:
	TestObject(CGHScene& scene)
		:GameObject(scene)
	{

	}
	virtual ~TestObject()=default;

private:
	virtual void Update(float delta) override;
	virtual void Init() override;
};