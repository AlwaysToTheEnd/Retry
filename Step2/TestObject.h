#pragma once
#include "GameObject.h"

class ComTransform;
class ComAnimator;

class TestObject :public GameObject
{
public:
	TestObject()=default;
	virtual ~TestObject()=default;

	virtual void Init() override;
	virtual void Update() override;
private:
	ComAnimator* ani;
};