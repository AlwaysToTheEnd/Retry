#include "UIObject.h"

void UIObject::UIOn()
{
	GameObject::SetActive(true, true);
}

void UIObject::UIOff()
{
	GameObject::SetActive(false, true);
}
