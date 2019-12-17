#pragma once
#include <memory>
#include "IComponent.h"
#include "DX12RenderClasses.h"
#include "AnimationStructs.h"
#include "AnimationTree.h"

struct RenderFont;

namespace AniTree
{
	class AnimationTree;
}

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

	static void GetMeshNames(std::vector<std::string>& out);
	bool SelectMesh(const std::string& name);
	const std::string& GetCurrMeshName() const { return m_CurrMeshName; }

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
		, m_CurrTick(0)
	{
		if (m_SkinnedDatas == nullptr)
		{
			m_SkinnedDatas = skinnedDatas;
			m_ReservedAniBone = reservedAniBone;
		}
	}

	virtual void Update() override;

	static void GetSkinNames(std::vector<std::string>& out);
	unsigned int GetAniEndTime(const std::string& name) const { return m_CurrSkinnedData->GetClipEndTime(name); }
	int GetBoneMatStoredIndex() const { return m_BoneMatStoredIndex; }
	void GetAniNames(std::vector<std::string>& out) const;
	AniTree::AnimationTree* GetAnimationTree() { return m_AniTree.get(); }
	
	void SetAnimationTree(bool value);
	bool SelectSkin(const std::string& name);
	bool SelectAnimation(const std::string& name);

private:
	static const std::unordered_map<std::string, Ani::SkinnedData>* m_SkinnedDatas;
	static std::vector<AniBoneMat>*									m_ReservedAniBone;

	std::unique_ptr<AniTree::AnimationTree>	m_AniTree;
	const Ani::SkinnedData*					m_CurrSkinnedData;
	unsigned long long						m_CurrTick;
	std::string								m_CurrAniName;
	int										m_BoneMatStoredIndex;
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

class ComFont :public IComponent
{
public:
	ComFont(GameObject& gameObject, int ID, 
		std::vector<RenderFont>* reservedFonts)
		: IComponent(COMPONENTTYPE::COM_FONT, gameObject, ID)
	{
		if (m_ReservedFonts == nullptr)
		{
			m_ReservedFonts = reservedFonts;
		}
	}

	virtual void Update() override;
private:
	static std::vector<RenderFont>* m_ReservedFonts;


};