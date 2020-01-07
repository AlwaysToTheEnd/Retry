#pragma once
#include <memory>
#include "DeviceObject.h"
#include "DX12RenderClasses.h"
#include "AnimationStructs.h"
#include "AnimationTree.h"

struct RenderFont;

namespace AniTree
{
	class AnimationTree;
}

class DOMesh :public DeviceObject
{
public:
	DOMesh(CGHScene& scene, GameObject* const parent, unsigned int hashCode)
		: DeviceObject(scene, parent, hashCode)
	{
	
	}
	virtual ~DOMesh() = default;

	virtual void Update(float delta) override {};

	void GetMeshNames(std::vector<std::string>& out);
	const MeshWorkFunc* GetDeviceMeshWorks() { return m_MeshWorks; }
	bool SelectMesh(const std::string& name);
	const std::string& GetCurrMeshName() const { return m_CurrMeshName; }

private:
	static const std::unordered_map<std::string, MeshObject>*	m_Meshs;
	static const MeshWorkFunc*									m_MeshWorks;

	const MeshObject* m_CurrMesh;
	std::string m_CurrMeshName;
};

class DOAnimator :public DeviceObject
{
public:
	DOAnimator(CGHScene& scene, GameObject* const parent, unsigned int hashCode)
		: DeviceObject(scene, parent, hashCode)
		, m_AniTree(nullptr)
		, m_BoneMatStoredIndex(-1)
		, m_CurrSkinnedData(nullptr)
		, m_CurrTick(0)
	{
		if (m_SkinnedDatas == nullptr)
		{
			m_SkinnedDatas = skinnedDatas;
			m_AnimationTrees = aniTreeDatas;
			m_ReservedAniBone = reservedAniBone;
		}
	}
	virtual ~DOAnimator() = default;

	virtual void Update(float delta) override;

	void GetSkinNames(std::vector<std::string>& out);
	unsigned int GetAniEndTime(const std::string& name) const { return m_CurrSkinnedData->GetClipEndTime(name); }
	const Ani::SkinnedData* GetSkinnedData(const std::string& name) const { return &m_SkinnedDatas->find(name)->second; }
	int GetBoneMatStoredIndex() const { return m_BoneMatStoredIndex; }
	void GetAniNames(std::vector<std::string>& out) const;
	
	bool SelectSkin(const std::string& name);
	bool SelectAnimation(const std::string& name);

	bool IsRegisteredTree(const AniTree::AnimationTree* tree) const;
	void SetAnimationTree(AniTree::AnimationTree* tree);
	void SetAnimationTree(const std::string& fileName);
	void GetAnimationTreeNames(std::vector<std::string>& out) const;
	AniTree::AnimationTree* GetAnimationTree() { return m_AniTree; }
	void SaveAnimationTree(const std::wstring& filePath, const std::string& fileName, AniTree::AnimationTree* tree);

private:
	static const std::unordered_map<std::string, Ani::SkinnedData>*						m_SkinnedDatas;
	static std::unordered_map<std::string,std::unique_ptr<AniTree::AnimationTree>>*	m_AnimationTrees;
	static std::vector<AniBoneMat>*														m_ReservedAniBone;

	AniTree::AnimationTree*					m_AniTree;
	const Ani::SkinnedData*					m_CurrSkinnedData;
	unsigned long long						m_CurrTick;
	std::string								m_CurrAniName;
	int										m_BoneMatStoredIndex;
};

class DORenderer :public DeviceObject
{
public:
	DORenderer(CGHScene& scene, GameObject* const parent, unsigned int hashCode)
		: DeviceObject(scene, parent, hashCode)
		, m_RenderInfo(RENDER_NONE)
	{
		if (m_ReservedRenderObjects == nullptr)
		{
			m_ReservedRenderObjects = reservedRenderObjects;
		}
	}
	virtual ~DORenderer() = default;

	virtual void Update(float delta) override;
	void SetRenderInfo(const RenderInfo& info) 
	{ 
		m_RenderInfo = info; 
	}
	const RenderInfo& GetRenderInfo() { return m_RenderInfo; }

private:
	void RenderMesh();

private:
	RenderInfo	m_RenderInfo;
	static std::vector<RenderInfo>* m_ReservedRenderObjects;
};

class DOFont :public DeviceObject
{
public:
	DOFont(CGHScene& scene, GameObject* const parent, unsigned int hashCode)
		: DeviceObject(scene, parent, hashCode)
		, m_Pos(0,0,0)
		, m_FontHeight(-1)
		, m_Color(0,0,0,1)
		, m_DrawSize(0,0)
		, m_Benchmark(RenderFont::FONTBENCHMARK::LEFT)
	{
		if (m_ReservedFonts == nullptr)
		{
			m_ReservedFonts = reservedFonts;
		}
	}
	virtual ~DOFont() = default;

	virtual void Update(float delta) override;
	void SetFont(std::wstring fontName) { m_FontName = fontName; }
	void SetBenchmark(RenderFont::FONTBENCHMARK mark) { m_Benchmark = mark; }

public:
	std::wstring		m_Text;
	int					m_FontHeight;
	physx::PxVec2		m_DrawSize;
	physx::PxVec3		m_Pos;
	physx::PxVec4		m_Color;

private:
	RenderFont::FONTBENCHMARK		m_Benchmark;
	static std::vector<RenderFont>*	m_ReservedFonts;
	std::wstring					m_FontName;
};