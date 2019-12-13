#pragma once
#include "GameObject.h"

class ComTransform;

class TestObject :public GameObject
{
public:
	TestObject()=default;
	virtual ~TestObject()=default;

	virtual void Init() override;
	virtual void Update() override;
private:
	ComTransform* m_Transform;
};