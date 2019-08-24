#include "GameObject.h"
#include "d3dApp.h"

IComponent* GameObject::AddComponent(COMPONENTTYPE type)
{
	m_components.push_back(GETAPP->CreateComponent(type, m_transform));

	return m_components.back().get();
}
