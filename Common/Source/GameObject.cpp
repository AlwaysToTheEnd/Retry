#include "GameObject.h"
#include "d3dApp.h"

void GameObject::AddComponent(D3DApp* app, COMPONENTTYPE type)
{
	m_components.push_back(app->CreateComponent(type, m_transform));
}
