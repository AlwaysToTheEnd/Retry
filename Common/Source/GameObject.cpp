#include "GameObject.h"
#include "PhysicsDO.h"
#include "GraphicDO.h"
#include "CGHScene.h"
#include "d3dApp.h"


GameObject::GameObject(CGHScene& scene, GameObject* parent, const char* typeName)
	: m_Scene(scene)
	, m_Parent(parent)
	, m_TypeName(typeName)
	, m_IsActive(true)
	, m_State(CLICKEDSTATE::NONE)
{
	
}

void GameObject::SetActive(bool value, bool components)
{
	m_IsActive = value;

	if (components)
	{
		for (auto& it : m_Components)
		{
			it->SetActive(value, components);
		}
	}
}

void GameObject::SetParent(GameObject* parent)
{
	if (m_Parent)
	{
		m_Parent->ExceptComponent(this);
	}
	
	m_Parent = parent;

	if (m_Parent)
	{
		for (auto& it : m_Parent->m_Components)
		{
			if (it == this)
			{
				return;
			}
		}

		m_Parent->m_Components.push_back(this);
	}
}

const GameObject* const GameObject::GetConstructor() const
{
	if (m_Parent)
	{
		return m_Parent->GetConstructor();
	}
	else
	{
		return this;
	}
}

void GameObject::InitObject(GameObject* object)
{
	m_Scene.AddGameObject(object);
}

void GameObject::Delete()
{
	for (auto& it : m_Components)
	{
		it->Delete();
	}

	if (m_Parent)
	{
		m_Parent->ExceptComponent(this);
	}

	m_Scene.DeleteGameObject(this);
}

void GameObject::ExceptComponent(GameObject* com)
{
	for (auto& it : m_Components)
	{
		if (it == com)
		{
			it = m_Components.back();
			m_Components.pop_back();
			break;
		}
	}
}
