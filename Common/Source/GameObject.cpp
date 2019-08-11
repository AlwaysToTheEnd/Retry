#include "GameObject.h"
#include "d3dApp.h"

void GameObject::AddComponent(COMPONENTTYPE type)
{
	m_components.push_back(D3DApp::GetApp()->CreateComponent(type, m_transform));
}
