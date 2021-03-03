#pragma once
#include <d3d12.h>
#include <d3dx12.h>
#include <wrl.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>
#include <algorithm>
#include <functional>

#include "IGraphicDevice.h"
#include "DX12/DX12FontMG.h"
#include "DX12/DX12IndexManagementBuffer.h"
#include "DX12/DX12TextureBuffer.h"
#include "DX12/DX12MeshSet.h"
#include "DX12/DX12UploadBuffer.h"
#include "DX12/DX12RenderClasses.h"
#include "DX12/DX12PSOController.h"
#include "DX12/DX12SwapChain.h"
#include "DX12/DX12MeshComputeCulling.h"

#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

class DX12DrawSetNormalMesh;
class DX12DrawSetSkinnedMesh;
class DX12DrawSetHeightField;
class DX12DrawSetPointBase;
class DX12DrawSetLight;
class DX12DrawSetUI;

using Microsoft::WRL::ComPtr;

class GraphicDX12 final : public IGraphicDevice
{
public:
	GraphicDX12();
	virtual ~GraphicDX12() override;

private:
	virtual void	Update(const CGHScene& scene, float delta) override;
	virtual int		GetPixelFuncID(const physx::PxVec2& clientPos) override;
	virtual void	Draw() override;
	virtual bool	Init(HWND hWnd, UINT windowWidth, UINT windowHeight) override;
	virtual void	OnResize() override;
	virtual void*	GetDevicePtr() override { return m_D3dDevice.Get(); }
	virtual void	GetWorldRay(physx::PxVec3& origin, physx::PxVec3& ray) const override;
	virtual void	CreateScene(const CGHScene& scene) override {} //#TODO

	virtual void	RegisterDeviceObject(CGHScene& scene, DeviceObject* gameObject) override;
	virtual void	UnRegisterDeviceObject(CGHScene& scene, DeviceObject* gameObject) override;

public: // Used from DeviceObject Init
	virtual const std::unordered_map<std::string, MeshObject>* GetMeshDataMap(CGH::MESH_TYPE type) override;

	virtual bool	CreateMesh(const std::string& meshName, MeshObject& meshinfo, CGH::MESH_TYPE type, unsigned int numVertices, const void* vertices, const std::vector<unsigned int>& indices) override;
	virtual bool	CreateMaterials(const std::vector<std::string>& materialNames, const std::vector<Material>& materials) override;
	virtual bool	EditMesh(const std::string& meshName, const std::vector<Vertex>& vertices) override;
	virtual bool	EditMaterial(const std::string& materialName, const Material& material) override;

	virtual int		GetTextureIndex(const std::wstring& group, const std::string& textureName) override;
	virtual int		GetRenderedObjectIDFromMousePos() override { return m_RenderedObjectIDFromMousePos; }
	virtual void	ReComputeHeightField(const std::string& name, physx::PxVec3 scale) override;

private: // Used Function by ReadyWorks 
	virtual void	LoadTextureFromFolder(const std::vector<std::wstring>& targetTextureFolders) override;
	virtual void	LoadMeshAndMaterialFromFolder(const std::vector<std::wstring>& targetMeshFolders) override;
	virtual void	LoadFontFromFolder(const std::vector<std::wstring>& targetFontFolders) override;
	virtual void	LoadAniTreeFromFolder(const std::wstring& targetFolder) override;
	virtual void	ReadyWorksEnd() override;

private: // Device Base Functions
	void FlushCommandQueue();
	void CreateCommandObject();

private: // Base object Builds
	void BuildDrawSets();
	void BuildDepthStencilAndBlendsAndRasterizer();

private: // Used in frame update
	void UpdateMainPassCB(float delta);
	void UpdateObjectCB();

private:
	D3D12_VIEWPORT						m_ScreenViewport;
	D3D12_RECT							m_ScissorRect;
	std::wstring						m_MainWndCaption = L"DX12";
	D3D_DRIVER_TYPE						m_D3dDriverType = D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_UNKNOWN;

	ComPtr<ID3D12Device>				m_D3dDevice;

	ComPtr<ID3D12Fence>					m_Fence;
	UINT64								m_CurrentFence = 0;

	ComPtr<ID3D12CommandQueue>			m_CommandQueue;
	ComPtr<ID3D12CommandAllocator>		m_DirectCmdListAlloc;
	ComPtr<ID3D12GraphicsCommandList>	m_CommandList;

	const UINT							m_NumFrameResource = 1;
	UINT								m_CurrFrame = 0;
	std::unique_ptr<DX12SwapChain>		m_Swap;
	DXGI_FORMAT							m_BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	physx::PxVec3						m_RayOrigin;
	physx::PxVec3						m_Ray;

	int									m_RenderedObjectIDFromMousePos = -1;

	DX12_COMPUTE_CULLING_FRUSTUM		m_BaseFrustum;

private:
	std::unique_ptr<DX12PSOController>										m_PSOCon;

	std::vector<ComPtr<ID3D12CommandAllocator>>								m_CmdListAllocs;
	std::unique_ptr<DX12UploadBuffer<DX12PassConstants>>					m_PassCB;

	std::unordered_map<std::wstring, std::unique_ptr<DX12TextureBuffer>>	m_TextureBuffers;
	std::unique_ptr<DX12IndexManagementBuffer<Material>>					m_Materials;

	std::unique_ptr<DX12MeshSet<Vertex>>									m_NormalMeshSet;
	std::unique_ptr<DX12MeshSet<SkinnedVertex>>								m_SkinnedMeshSet;
	std::unique_ptr<DX12MeshSet<float>>										m_HeightFieldMeshSet;

	std::unique_ptr<DX12DrawSetNormalMesh>									m_NormalMeshDrawSet;
	std::unique_ptr<DX12DrawSetSkinnedMesh>									m_SkinnedMeshDrawSet;
	std::unique_ptr<DX12DrawSetHeightField>									m_HeightFieldMeshDrawSet;
	std::unique_ptr<DX12DrawSetLight>										m_LightDrawSet;
	std::unique_ptr<DX12DrawSetPointBase>									m_PointBaseDrawSet;
	std::unique_ptr<DX12DrawSetUI>											m_UIDrawSet;
	std::unique_ptr<DX12FontManager>										m_FontManager;
};
