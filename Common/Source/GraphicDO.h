#pragma once
#include "DeviceObject.h"
#include "GraphicBase.h"
#include "AnimationStructs.h"
#include "AnimationTree.h"
#include "Mouse.h"

struct RenderFont;
#define FONTRENDERERID(T) -(T+2)

namespace AniTree
{
	class AnimationTree;
}

class DORenderMesh :public DeviceObject
{
public:
	DORenderMesh(CGHScene& scene, GameObject* parent, const char* typeName)
		: DeviceObject(scene, parent, typeName)
		, m_CurrMeshType(CGH::NORMAL_MESH)
		, m_CurrMesh(nullptr)
	{
	
	}
	virtual ~DORenderMesh() = default;

	void				GetMeshNames(CGH::MESH_TYPE type, std::vector<std::string>& out);
	const std::string&	GetCurrMeshName() const { return m_CurrMeshName; }
	CGH::MESH_TYPE		GetCurrMeshType() const {}

	bool				SelectMesh(CGH::MESH_TYPE type ,const std::string& name);
	void				ReComputeHeightField(physx::PxVec3 scale);

private:
	virtual void Update(float delta) override {}
	virtual void Init(IGraphicDevice* graphicDevice, ISoundDevice*, PhysX4_1*);

private:
	static const std::unordered_map<std::string, MeshObject>*		m_Meshs[CGH::MESH_TYPE_COUNT];
	static std::function<void(const std::string&, physx::PxVec3)>	m_ReComputeHeightFieldFunc;

	std::string			m_CurrMeshName;
	CGH::MESH_TYPE		m_CurrMeshType;
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
		, m_CurrTime(0)
	{
		
	}
	virtual ~DOAnimator() = default;

	void					GetAnimationTreeNames(std::vector<std::string>& out) const;
	AniTree::AnimationTree* GetCurrAnimationTree() { return m_AniTree; }
	void					GetSkinNames(std::vector<std::string>& out) const;
	double					GetAniEndTime(const std::string& name) const { return m_CurrSkinnedData->GetClipEndTime(name); }
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
	virtual void Init(IGraphicDevice* graphicDevice, ISoundDevice*, PhysX4_1*);

private:
	static const std::unordered_map<std::string, Ani::SkinnedData>*						m_SkinnedDatas;
	static std::unordered_map<std::string,std::unique_ptr<AniTree::AnimationTree>>*		m_AnimationTrees;
	static std::vector<AniBoneMat>*														m_ReservedAniBone;

	AniTree::AnimationTree*					m_AniTree;
	const Ani::SkinnedData*					m_CurrSkinnedData;
	double									m_CurrTime;
	std::string								m_CurrAniName;
	int										m_BoneMatStoredIndex;
};

struct RenderFuncFromMouse
{
	std::vector<std::function<void()>>								funcs;
	std::vector<DirectX::Mouse::ButtonStateTracker::ButtonState>	states;
	std::vector<DirectX::MOUSEBUTTONINDEX>							buttonIndices;
};

class DORenderer :public DeviceObject
{
public:
	DORenderer(CGHScene& scene, GameObject* parent, const char* typeName)
		: DeviceObject(scene, parent, typeName)
		, m_RenderInfo(RENDER_NONE)
		, m_IsPixelFuncRenderer(false)
	{
	}
	virtual ~DORenderer() = default;

	void		DoFuncFromMouse(DirectX::Mouse::ButtonStateTracker::ButtonState state, DirectX::MOUSEBUTTONINDEX index);

	void		SetRenderInfo(const RenderInfo& info) { m_RenderInfo = info; }
	void		SetPixelFuncRenderer(bool value) { m_IsPixelFuncRenderer = value; }
	void		AddPixelFunc(std::function<void()> func, 
				DirectX::Mouse::ButtonStateTracker::ButtonState state, DirectX::MOUSEBUTTONINDEX index);

	void		ClearPixelFunc() { m_Funcs = nullptr; }
	RenderInfo&	GetRenderInfo() { return m_RenderInfo; }

private:
	virtual void Update(float delta) override;
	virtual void Init(IGraphicDevice* graphicDevice, ISoundDevice*, PhysX4_1*);

private:
	void		 RenderMesh();

private:
	static std::vector<RenderInfo>*			m_ReservedRenderObjects;
	std::unique_ptr<RenderFuncFromMouse>	m_Funcs;
	bool									m_IsPixelFuncRenderer;
	RenderInfo								m_RenderInfo;
};

class DOFont :public DeviceObject
{
public:
	DOFont(CGHScene& scene, GameObject* parent, const char* typeName)
		: DeviceObject(scene, parent, typeName)
		, m_Pos(0,0,0)
		, m_FontHeight(-1)
		, m_Color(1,1,1,1)
		, m_DrawSize(0,0)
		, m_Benchmark(0,0)
	{
	}
	virtual ~DOFont() = default;

	void	DoFuncFromMouse(DirectX::Mouse::ButtonStateTracker::ButtonState state, DirectX::MOUSEBUTTONINDEX index);

	void	SetFont(std::wstring fontName) { m_FontName = fontName; }
	void	SetBenchmark(const physx::PxVec2& uv) { m_Benchmark = uv; }
	void	AddPixelFunc(std::function<void()> func,
			DirectX::Mouse::ButtonStateTracker::ButtonState state, DirectX::MOUSEBUTTONINDEX index);

	int		GetFontRenderID() { return FONTRENDERERID(GetDeviceOBID()); }
	void	ClearPixelFunc() { m_Funcs = nullptr; }

private:
	virtual void Update(float delta) override;
	virtual void Init(IGraphicDevice* graphicDevice, ISoundDevice*, PhysX4_1*);

public:
	physx::PxVec4		m_Color;
	physx::PxVec2		m_DrawSize;
	physx::PxVec3		m_Pos;
	int					m_FontHeight;
	std::wstring		m_Text;

private:
	static std::vector<RenderFont>*			m_ReservedRenderFonts;
	std::unique_ptr<RenderFuncFromMouse>	m_Funcs;
	physx::PxVec2							m_Benchmark;
	std::wstring							m_FontName;
};