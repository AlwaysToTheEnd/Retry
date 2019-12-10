#include "GraphicComponent.h"
#include "GameObject.h"

const std::unordered_map<std::string, MeshObject>* ComMesh::m_Meshs=nullptr;

bool ComMesh::SelectMesh(std::string name)
{
	auto iter = m_Meshs->find(name);
	if (iter == m_Meshs->end())
	{
		m_CurrMesh = nullptr;
		return false;
	}
	
	m_CurrMesh = &iter->second;
	return true;
}
