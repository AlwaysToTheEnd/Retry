#pragma once
#include "IComponent.h"
#include "DX12RenderClasses.h"
#include "AnimationStructs.h"

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

	bool SelectMesh(std::string& name);
	const std::string& GetMeshName() const { return m_CurrMeshName; }

private:
	static const std::unordered_map<std::string, MeshObject>* m_Meshs;

	const MeshObject* m_CurrMesh;
	std::string m_CurrMeshName;
};

class ComAnimator :public IComponent
{
public:
	ComAnimator(GameObject& gameObject, int ID,
		const std::unordered_map<std::string, Ani::SkinnedData>* skinnedDatas,
		std::vector<AniBoneMat>* reservedAniBone)
		: IComponent(COMPONENTTYPE::COM_ANIMATOR, gameObject, ID)
		, m_BoneMatStoredIndex(-1)
		, m_CurrSkinnedData(nullptr)
		, m_currTick(0)
	{
		if (skinnedDatas == nullptr)
		{
			m_SkinnedDatas = skinnedDatas;
			m_ReservedAniBone = reservedAniBone;
		}
	}

	virtual void Update() override;

	bool SelectSkin(std::string& name);
	bool SelectAnimation(std::string& name);
	int GetBoneMatStoredIndex() const { return m_BoneMatStoredIndex; }

private:
	static const std::unordered_map<std::string, Ani::SkinnedData>* m_SkinnedDatas;
	static std::vector<AniBoneMat>*									m_ReservedAniBone;

	const Ani::SkinnedData*	m_CurrSkinnedData;
	std::string				m_CurrAniName;
	int						m_BoneMatStoredIndex;
	unsigned long long		m_currTick;
};

class ComRenderer :public IComponent
{
public:
	ComRenderer(GameObject& gameObject, int ID,
		std::vector<RenderInfo>* reservedRenderObjects)
		: IComponent(COMPONENTTYPE::COM_RENDERER, gameObject, ID)
	{
		if (m_ReservedRenderObjects == nullptr)
		{
			m_ReservedRenderObjects = reservedRenderObjects;
		}
	}

	virtual void Update() override;

private:
	static std::vector<RenderInfo>* m_ReservedRenderObjects;
};