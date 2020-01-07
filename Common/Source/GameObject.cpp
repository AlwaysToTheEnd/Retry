#include "GameObject.h"
#include "PhysicsDO.h"
#include "GraphicDO.h"
#include "CGHScene.h"
#include "d3dApp.h"


GameObject::GameObject(CGHScene& scene, GameObject* const parent, unsigned int hashCode, bool enrollment)
	: m_Scene(scene)
	, m_Parent(parent)
	, m_HashCode(hashCode)
{
	if (enrollment)
	{
		m_Scene.AddGameObject(this);
	}
}

void GameObject::SetAllComponentActive(bool value)
{
	for (auto& it : m_Components)
	{
		it->SetActive(value);
	}
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
