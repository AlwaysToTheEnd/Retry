#include "IComponent.h"
#include "GameObject.h"
#include "CGHScene.h"

IComponent::IComponent(COMPONENTTYPE type, GameObject& gameObject, int ID) 
	: m_TargetGameObject(gameObject)
	, m_Type(type)
	, m_ID(ID)
	, m_IsActive(true)
{

}

IComponent::~IComponent()
{
	m_TargetGameObject.GetScene().ComponentDeleteManaging(m_Type, m_ID);
}
