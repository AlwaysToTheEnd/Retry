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
#include "XFileParser.h"
#include "DX12/DX12FontMG.h"
#include "DX12/DX12IndexManagementBuffer.h"
#include "DX12/DX12UploadBuffer.h"
#include "DX12/DX12RenderClasses.h"
#include "DX12/PSOController.h"
#include "DX12/DX12SwapChain.h"

#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

class DX12TextureBuffer;
class DX12DrawSetNormalMesh;
class DX12DrawSetSkinnedMesh;

using Microsoft::WRL::ComPtr;

class GraphicDX12 final : public IGraphicDevice
{
	enum
	{
		P1_OBJECT_SRV,
		P1_PASS_CB,
		P1_TEXTURE_TABLE,
		P1_ROOT_COUNT
	};

	enum
	{
		UI_PASS_CB,
		UI_UIPASS_CB,
		UI_TEXTURE_TABLE,
		UI_ROOT_COUNT
	};

	enum DX12_RENDER_TYPE
	{
		DX12_RENDER_TYPE_DYNAMIC_MESH,
		DX12_RENDER_TYPE_POINT,
		DX12_RENDER_TYPE_UI,
		DX12_RENDER_TYPE_COUNT
	};

	struct FrameResource
	{
		FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount)
		{
			ThrowIfFailed(device->CreateCommandAllocator(
				D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(cmdListAlloc.GetAddressOf())));

			passCB = std::make_unique<DX12UploadBuffer<PassConstants>>(device, passCount, true);
			uiPassCB = std::make_unique<DX12UploadBuffer<CGH::GlobalOptions::UIOption>>(device, 1, true);

			for (int i = 0; i < DX12_RENDER_TYPE_COUNT; i++)
			{
				meshObjectCB[i]= std::make_unique<DX12UploadBuffer<ObjectConstants>>(device, objectCount, true);
			}

			pointCB = std::make_unique<DX12UploadBuffer<OnlyTexObjectConstants>>(device, objectCount, false);
		}

		FrameResource(const FrameResource& rhs) = delete;
		FrameResource& operator=(const FrameResource& rhs) = delete;

		ComPtr<ID3D12CommandAllocator> cmdListAlloc;

		std::unique_ptr<DX12UploadBuffer<PassConstants>> passCB = nullptr;
		std::unique_ptr<DX12UploadBuffer<CGH::GlobalOptions::UIOption>> uiPassCB = nullptr;
		std::unique_ptr<DX12UploadBuffer<ObjectConstants>> meshObjectCB[DX12_RENDER_TYPE_COUNT];
		std::unique_ptr<DX12UploadBuffer<OnlyTexObjectConstants>> pointCB = nullptr;
	};

	struct DynamicBuffer
	{
		DynamicBuffer(ID3D12Device* device, unsigned int _renderID, unsigned int _numVertex, unsigned int _numIndex)
		{
			dynamicIndexBuffer = std::make_unique<DX12UploadBuffer<UINT>>(device, _numIndex, false);
			dynamicVertexBuffer = std::make_unique<DX12UploadBuffer<Vertex>>(device, _numVertex, false);
			dynamicBufferInfo = std::make_unique<DynamicBufferInfo>(_renderID, _numVertex, _numIndex, 
				dynamicVertexBuffer->GetMappedData(), dynamicIndexBuffer->GetMappedData());
		}

		std::unique_ptr<DynamicBufferInfo>			dynamicBufferInfo;
		std::unique_ptr<DX12UploadBuffer<Vertex>>		dynamicVertexBuffer;
		std::unique_ptr<DX12UploadBuffer<unsigned int>>	dynamicIndexBuffer;
	};

public:
	GraphicDX12();
	virtual ~GraphicDX12() override;

private:
	virtual void	Update(const CGHScene& scene) override;
	virtual void	Draw() override;
	virtual bool	Init(HWND hWnd, UINT windowWidth, UINT windowHeight) override;
	virtual void	OnResize() override;
	virtual void*	GetDevicePtr() override { return m_D3dDevice.Get(); }
	virtual void	GetWorldRay(physx::PxVec3& origin, physx::PxVec3& ray) const override;
	virtual void	CreateScene(const CGHScene& scene) override {} //#TODO

	virtual void	RegisterDeviceObject(CGHScene& scene, DeviceObject* gameObject) override;
	virtual void	UnRegisterDeviceObject(CGHScene& scene, DeviceObject* gameObject) override;

public: // Used from DeviceObject Init
	virtual const std::unordered_map<std::string, MeshObject>* GetMeshDataMap() override;

	virtual bool	CreateMesh(const std::string& meshName, MeshObject& meshinfo, const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices) override;
	virtual bool	CreateMaterials(const std::vector<std::string>& materialNames, const std::vector<Material>& materials) override;
	virtual bool	EditMesh(const std::string& meshName, const std::vector<Vertex>& vertices) override;
	virtual bool	EditMaterial(const std::string& materialName, const Material& material) override;

	virtual bool	CreateDynamicVIBuffer(unsigned int vertexNum, unsigned int indexNum, DynamicBufferInfo** out);
	virtual void	EditDynamicVIBuffer(const DynamicBufferInfo* dvi, DYNAMIC_BUFFER_EDIT_MOD mode, const std::vector<float>& inputDatas);
	virtual void	SaveAndMergeDynamicVIBufferToDefaultVertexBuffer(const std::string& meshName, const DynamicBufferInfo* dvi) override;
	virtual void	ReleaseDynamicVIBuffer(const DynamicBufferInfo* dvi) override;

	virtual int		GetTextureIndex(const std::string& textureName) override;

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
	void BuildFrameResources();
	void BuildRootSignature();
	void BuildShadersAndInputLayout();
	void BuildDepthStencilAndBlendsAndRasterizer();
	void BuildUploadBuffers();

private: // Used in frame.
	void UpdateMainPassCB();
	void UpdateObjectCB();

private:
	void DrawObject(DX12_RENDER_TYPE type);
	void DrawDynamicMehs();
	void DrawPointObjects();
	void DrawUIs();

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

	std::unique_ptr<DX12SwapChain>		m_Swap;
	DXGI_FORMAT							m_BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT							m_DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	physx::PxVec3						m_RayOrigin;
	physx::PxVec3						m_Ray;

private:
	std::unique_ptr<PSOController>						m_PSOCon;

	PassConstants										m_MainPassCB;
	std::unique_ptr<FrameResource>						m_FrameResource;
	std::unique_ptr<DX12FontManager>					m_FontManager;

	std::unique_ptr<DX12TextureBuffer>						m_TextureBuffer;
	std::unique_ptr<DX12TextureBuffer>						m_UITextureBuffer;
	std::unique_ptr<DX12IndexManagementBuffer<Material>>	m_Materials;

	std::unique_ptr<DX12DrawSetNormalMesh>					m_NormalMeshDrawSet;
	std::unique_ptr<DX12DrawSetSkinnedMesh>					m_SkinnedMeshDrawSet;

private:
	std::vector<ObjectConstants>					m_RenderObjects[DX12_RENDER_TYPE_COUNT];
	std::vector<const SubmeshData*>					m_RenderObjectsSubmesh[DX12_RENDER_TYPE_COUNT];
	unsigned int									m_NumRenderPointObjects;
	unsigned int									m_NumRenderUIs;

private:
	std::unique_ptr<DX12UploadBuffer<B_P_Vertex>>		m_Box_Plane_Vertices;
	std::unique_ptr<DX12UploadBuffer<UIInfomation>>		m_UIInfomation;

	std::vector<std::unique_ptr<DynamicBuffer>>		m_DynamicBuffers;
};
