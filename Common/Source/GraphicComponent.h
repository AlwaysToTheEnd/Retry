#pragma once
#include "DX12RenderClasses.h"
#include "IComponent.h"

class ComMesh :public IComponent
{
public:
	ComMesh(GameObject& gameObject, int ID,
		const std::unordered_map<std::string, MeshObject>* meshs)
		: IComponent(COMPONENTTYPE::COM_MESH ,gameObject, ID)
		, m_CurrMesh(nullptr)
	{
		if (m_Meshs == nullptr)
		{
			m_Meshs = meshs;
		}
	}

	virtual void Update() override {};
	bool SelectMesh(std::string name);

private:
	static const std::unordered_map<std::string, MeshObject>* m_Meshs;
	const MeshObject* m_CurrMesh;
};

class ComRenderer :public IComponent
{
public:
	ComRenderer(GameObject& gameObject, int ID);

	virtual void Update() override;

private:
};

class ComAnimater :public IComponent
{
public:
	ComAnimater(GameObject& gameObject, int ID);

	virtual void Update() override;

private:
};