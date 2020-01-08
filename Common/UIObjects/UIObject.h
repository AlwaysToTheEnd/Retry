#pragma once
#include "GameObject.h"


class UIObject :public GameObject
{
public:
	UIObject(CGHScene& scene, GameObject* parent, const char* typeName)
		: GameObject(scene, parent, typeName)
	{
	}
	virtual ~UIObject() = default;

	virtual void	UIOn();
	virtual void	UIOff();

protected:
	virtual void	SetActive(bool value, bool components = false) override { GameObject::SetActive(value, components); }

private:
};