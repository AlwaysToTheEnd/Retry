#pragma once
#include <windows.h>
#include "IDeviceObjectRegistration.h"
#include "GraphicBase.h"
#include "Vertex.h"
#include "AnimationStructs.h"
#include "AnimationTree.h"

class cCamera;
struct MeshObject;
class D3DApp;

class IGraphicDevice : public IDeviceObjectRegistration
{
	friend class D3DApp;
	friend class CGHScene;
public:
	IGraphicDevice() = default;
	virtual ~IGraphicDevice() = default;

protected:
	virtual bool Init(HWND hWnd, UINT windowWidth, UINT windowHeight) = 0;
	virtual void Update(const CGHScene& scene, float delta) = 0;
	virtual void Draw() = 0;
	virtual void* GetDevicePtr() = 0;
	virtual void OnResize() = 0;
	virtual void GetWorldRay(physx::PxVec3& origin, physx::PxVec3& ray) const = 0;
	virtual void CreateScene(const CGHScene& scene) = 0;

public:
	std::vector<RenderFont>* GetReservedRenderFontVector() { return &m_ReservedFonts; }
	std::vector<AniBoneMat>* GetReservedAniBoneMatVector() { return &m_ReservedAniBones; }
	std::vector<RenderInfo>* GetReservedRenderInfoVector() { return &m_ReservedRenderInfos; }

	std::unordered_map<std::string, Ani::SkinnedData>*							GetSkinnedDataMap() { return &m_SkinnedDatas; }
	std::unordered_map<std::string, std::unique_ptr<AniTree::AnimationTree>>*	GetAnimationTreeMap() { return &m_AniTreeDatas; }

public: // pure virtual funcs. 
	virtual const std::unordered_map<std::string, MeshObject>*					GetMeshDataMap(CGH::MESH_TYPE type) = 0;

	virtual bool	CreateMesh(const std::string& meshName, MeshObject& meshinfo, CGH::MESH_TYPE type, unsigned int numVertices ,const void* vertices, const std::vector<unsigned int>& indices) = 0;
	virtual bool	CreateMaterials(const std::vector<std::string>& materialNames, const std::vector<Material>& materials) = 0;
	virtual bool	EditMesh(const std::string& meshName, const std::vector<Vertex>& vertices) = 0;
	virtual bool	EditMaterial(const std::string& materialName, const Material& material) = 0;

	virtual int		GetTextureIndex(const std::string& textureName) = 0;
	virtual void	ReComputeHeightField(const std::string& name, physx::PxVec3 scale) = 0;

private: // used by D3DApp.
	physx::PxVec2	GetClientSize() const { return physx::PxVec2(m_ClientWidth, m_ClientHeight); }
	void			SetCamera(cCamera* camera) { m_CurrCamera = camera; }
	void			ReservedWorksClear();
	void			SetClientSize(UINT width, UINT height) { m_ClientWidth = width, m_ClientHeight = height; }
	void			ReadyWorks(	const std::vector<std::wstring>& targetTextureFolders,
								const std::vector<std::wstring>& targetMeshFolders,
								const std::vector<std::wstring>& targetFontFolders,
								const std::wstring& targetAniTreeFolder)
	{
		LoadTextureFromFolder(targetTextureFolders);
		LoadMeshAndMaterialFromFolder(targetMeshFolders);
		LoadFontFromFolder(targetFontFolders);
		LoadAniTreeFromFolder(targetAniTreeFolder);
		ReadyWorksEnd();
	}

protected:
	virtual void LoadTextureFromFolder(const std::vector<std::wstring>& targetTextureFolders) = 0;
	virtual void LoadMeshAndMaterialFromFolder(const std::vector<std::wstring>& targetMeshFolders) = 0;
	virtual void LoadFontFromFolder(const std::vector<std::wstring>& targetFontFolders) = 0;
	virtual void LoadAniTreeFromFolder(const std::wstring& targetFolder) = 0;
	virtual void ReadyWorksEnd() = 0;

protected:
	physx::PxMat44				m_ViewMatrix;
	physx::PxMat44				m_ProjectionMat;
	physx::PxMat44				m_OrthoProjectionMat;
	HWND						m_MainWndHandle = nullptr;
	int							m_ClientWidth = 700;
	int							m_ClientHeight = 700;
	cCamera*					m_CurrCamera = nullptr;

	std::vector<RenderFont>		m_ReservedFonts;
	std::vector<RenderInfo>		m_ReservedRenderInfos;

	std::vector<AniBoneMat>														m_ReservedAniBones;
	std::unordered_map<std::string, Ani::SkinnedData>							m_SkinnedDatas;
	std::unordered_map<std::string, std::unique_ptr<AniTree::AnimationTree>>	m_AniTreeDatas;
};

