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

void UIObject::SetUICollisionSize(DOUICollision* uiCol)
{
	uiCol->SetSize(m_Size/2);

	physx::PxVec2 offset = m_Size/2;

	uiCol->SetOffset(offset);
}
