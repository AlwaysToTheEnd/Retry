#pragma once
#include "GameObject.h"

class ComTransform;
class ComAnimator;
class ComFont;
class CGHScene;

class TestObject :public GameObject
{
public:
	TestObject(CGHScene& scene)
		:GameObject(scene)
	{

	}
	virtual ~TestObject()=default;

private:
	virtual void Init() override;
	virtual void Update() override;

	void TextChange();
private:
	ComFont*		font;
};