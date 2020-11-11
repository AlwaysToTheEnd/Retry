#include "UIObject.h"
#include "PhysicsDO.h"

void UIObject::UIOn()
{
	GameObject::SetActive(true, true);
}

void UIObject::UIOff()
{
	GameObject::SetActive(false, true);
}