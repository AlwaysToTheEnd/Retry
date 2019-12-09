#include "GameObject.h"
#include "d3dApp.h"

IComponent* GameObject::AddComponent(COMPONENTTYPE type)
{
	m_components[type].push_back(GETAPP->CreateComponent(type, *this));

	return m_components[type].back().get();
}
