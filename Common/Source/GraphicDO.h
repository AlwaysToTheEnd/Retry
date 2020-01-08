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
	DOMesh(CGHScene& scene, GameObject* parent, const char* typeName)
		: DeviceObject(scene, parent, typeName)
	{
	
	}
	virtual ~DOMesh() = default;

	void				SetDOMeshNeedInfoFromDevice(const MeshWorkFunc* funcs,
									const std::unordered_map<std::string, MeshObject>* meshs);

	void				GetMeshNames(std::vector<std::string>& out);
	const MeshWorkFunc* GetDeviceMeshWorks() { return m_MeshWorks; }
	const std::string&	GetCurrMeshName() const { return m_CurrMeshName; }

	bool				SelectMesh(const std::string& name);

private:
	virtual void Update(float delta) override {}
	virtual void Init() override {}

private:
	static const MeshWorkFunc*									m_MeshWorks;
	static const std::unordered_map<std::string, MeshObject>*	m_Meshs;

	std::string			m_CurrMeshName;
	const MeshObject*	m_CurrMesh;
};

class DOAnimator :public DeviceObject
{
public:
	DOAnimator(CGHScene& scene, GameObject* parent, const char* typeName)
		: DeviceObject(scene, parent, typeName)
		, m_AniTree(nullptr)
		, m_BoneMatStoredIndex(-1)
		, m_CurrSkinnedData(nullptr)
		, m_CurrTick(0)
	{
		
	}
	virtual ~DOAnimator() = default;

	void					SetDOAnimatorNeedInfoFromDevice(std::vector<AniBoneMat>* reservedAnibones,
								const std::unordered_map<std::string, Ani::SkinnedData>* skinnedDatas,
								std::unordered_map<std::string, std::unique_ptr<AniTree::AnimationTree>>* animationTrees);

	void					GetAnimationTreeNames(std::vector<std::string>& out) const;
	AniTree::AnimationTree* GetCurrAnimationTree() { return m_AniTree; }
	void					GetSkinNames(std::vector<std::string>& out) const;
	unsigned int			GetAniEndTime(const std::string& name) const { return m_CurrSkinnedData->GetClipEndTime(name); }
	const Ani::SkinnedData* GetSkinnedData(const std::string& name) const { return &m_SkinnedDatas->find(name)->second; }
	int						GetBoneMatStoredIndex() const { return m_BoneMatStoredIndex; }
	void					GetAniNames(std::vector<std::string>& out) const;
	bool					IsRegisteredTree(const AniTree::AnimationTree* tree) const;
	
	bool					SelectSkin(const std::string& name);
	bool					SelectAnimation(const std::string& name);

	void					SetAnimationTree(AniTree::AnimationTree* tree);
	void					SetAnimationTree(const std::string& fileName);
	void					SaveAnimationTree(const std::wstring& filePath, const std::string& fileName, AniTree::AnimationTree* tree);
	void					SaveAnimationTree(const std::wstring& filePath, const std::string& fileName, std::unique_ptr<AniTree::AnimationTree> tree);

private:
	virtual void Update(float delta) override;
	virtual void Init() override {}

private:
	static const std::unordered_map<std::string, Ani::SkinnedData>*						m_SkinnedDatas;
	static std::unordered_map<std::string,std::unique_ptr<AniTree::AnimationTree>>*		m_AnimationTrees;
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
	DORenderer(CGHScene& scene, GameObject* parent, const char* typeName)
		: DeviceObject(scene, parent, typeName)
		, m_RenderInfo(RENDER_NONE)
	{
		
	}
	virtual ~DORenderer() = default;

	void				SetDORenderNeedInfoFromDevice(std::vector<RenderInfo>* reservedRenderOBs) { m_ReservedRenderObjects = reservedRenderOBs; }

	void				SetRenderInfo(const RenderInfo& info) { m_RenderInfo = info; }
	
	const RenderInfo&	GetRenderInfo() { return m_RenderInfo; }

private:
	virtual void Update(float delta) override;
	virtual void Init() override {}

private:
	void		 RenderMesh();

private:
	static std::vector<RenderInfo>* m_ReservedRenderObjects;
	RenderInfo	m_RenderInfo;
};

class DOFont :public DeviceObject
{
public:
	DOFont(CGHScene& scene, GameObject* parent, const char* typeName)
		: DeviceObject(scene, parent, typeName)
		, m_Pos(0,0,0)
		, m_FontHeight(-1)
		, m_Color(0,0,0,1)
		, m_DrawSize(0,0)
		, m_Benchmark(RenderFont::FONTBENCHMARK::LEFT)
	{
	}
	virtual ~DOFont() = default;

	void			SetDOFontNeedInfoFromDevice(std::vector<RenderFont>* reservedFonts) { m_ReservedFonts = reservedFonts; }

	void			SetFont(std::wstring fontName) { m_FontName = fontName; }
	void			SetBenchmark(RenderFont::FONTBENCHMARK mark) { m_Benchmark = mark; }

private:
	virtual void Update(float delta) override;
	virtual void Init() override {}

public:
	physx::PxVec4		m_Color;
	physx::PxVec2		m_DrawSize;
	physx::PxVec3		m_Pos;
	int					m_FontHeight;
	std::wstring		m_Text;

private:
	static std::vector<RenderFont>*	m_ReservedFonts;
	RenderFont::FONTBENCHMARK		m_Benchmark;
	std::wstring					m_FontName;
};