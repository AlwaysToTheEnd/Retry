#include "IComponent.h"
#include "GameObject.h"

std::function<void(COMPONENTTYPE, int)> IComponent::m_InstanceDeleteManagingFunc = nullptr;

IComponent::IComponent(COMPONENTTYPE type, GameObject& gameObject, int ID) 
	: m_TargetGameObject(&gameObject)
	, m_Type(type)
	, m_ID(ID)
{

}

IComponent::~IComponent()
{
	m_InstanceDeleteManagingFunc(m_Type, m_ID);
}

void IComponent::SetInstanceDeleteManagingFunc(std::function<void(COMPONENTTYPE, int)> func)
{
	m_InstanceDeleteManagingFunc = func;
}
