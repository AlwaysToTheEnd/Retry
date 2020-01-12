#pragma once
#include <d3d12.h>
#include <d3dx12.h>
#include <wrl.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>
#include <algorithm>
#include <functional>

#include "IGraphicDevice.h"
#include "DX12FontMG.h"
#include "XFileParser.h"
#include "cIndexManagementBuffer.h"
#include "DX12RenderClasses.h"

#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

class cTextureBuffer;

using Microsoft::WRL::ComPtr;

template <typename T>
class UploadBuffer
{
public:
	UploadBuffer(ID3D12Device* device, UINT elementCount, bool isConstantBuffer)
	{
		m_ElementByteSize = sizeof(T);
		m_IsConstantBuffer = isConstantBuffer;
		m_NumElement = elementCount;

		if (isConstantBuffer)
		{
			m_ElementByteSize = (sizeof(T) + 255) & ~255;
		}

		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(m_ElementByteSize * elementCount),
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
			IID_PPV_ARGS(m_UploadBuffer.GetAddressOf())));

		ThrowIfFailed(m_UploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_MappedData)));
	}

	UploadBuffer(const UploadBuffer& rhs) = delete;
	UploadBuffer& operator=(const UploadBuffer& rhs) = delete;

	~UploadBuffer()
	{
		if (m_UploadBuffer)
		{
			m_UploadBuffer->Unmap(0, nullptr);
		}

		m_MappedData = nullptr;
	}

	ID3D12Resource* Resource()const
	{
		return m_UploadBuffer.Get();
	}

	void CopyData(int elementIndex, const T& data)
	{
		std::memcpy(&m_MappedData[elementIndex * m_ElementByteSize], &data, sizeof(T));
	}

	void CopyData(int elementIndex, const T* data)
	{
		std::memcpy(&m_MappedData[elementIndex * m_ElementByteSize], data, sizeof(T));
	}

	void CopyData(int numElement, int offsetIndex, const T* data)
	{
		assert(!m_IsConstantBuffer);
		std::memcpy(&m_MappedData[offsetIndex * m_ElementByteSize], data, sizeof(T) * numElement);
	}

	T*		GetMappedData() { return reinterpret_cast<T*>(m_MappedData); }
	UINT	GetElementByteSize() const { return m_ElementByteSize; }
	UINT	GetBufferSize() const { return m_ElementByteSize * m_NumElement; }
	UINT	GetNumElement() const { return m_NumElement; }

private:
	ComPtr<ID3D12Resource> m_UploadBuffer;
	BYTE* m_MappedData = nullptr;

	UINT	m_NumElement = 0; 
	UINT64	m_ElementByteSize = 0;
	bool	m_IsConstantBuffer = false;
};


class GraphicDX12 final : public IGraphicDevice
{
	enum
	{
		T1_MATERIAL_SRV,
		T1_PASS_CB,
		T1_OBJECT_CB,
		T1_TEXTURE_TABLE,
		T1_ANIBONE_CB,
		T1_ROOT_COUNT
	};

	enum
	{
		P1_OBJECT_SRV,
		P1_PASS_CB,
		P1_TEXTURE_TABLE,
		P1_ROOT_COUNT
	};

	enum DX12_RENDER_TYPE
	{
		DX12_RENDER_TYPE_NORMAL_MESH,
		DX12_RENDER_TYPE_DYNAMIC_MESH,
		DX12_RENDER_TYPE_SKINNED_MESH,
		DX12_RENDER_TYPE_POINT,
		DX12_RENDER_TYPE_COUNT
	};

	struct FrameResource
	{
		FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount, UINT aniBoneSetNum)
		{
			ThrowIfFailed(device->CreateCommandAllocator(
				D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(cmdListAlloc.GetAddressOf())));

			passCB = std::make_unique<UploadBuffer<PassConstants>>(device, passCount, true);

			for (int i = 0; i < DX12_RENDER_TYPE_COUNT; i++)
			{
				meshObjectCB[i]= std::make_unique<UploadBuffer<ObjectConstants>>(device, objectCount, true);
			}

			pointCB = std::make_unique<UploadBuffer<OnlyTexObjectConstants>>(device, objectCount, false);
			aniBoneMatBuffer = std::make_unique<UploadBuffer<AniBoneMat>>(device, aniBoneSetNum, true);
		}

		FrameResource(const FrameResource& rhs) = delete;
		FrameResource& operator=(const FrameResource& rhs) = delete;

		ComPtr<ID3D12CommandAllocator> cmdListAlloc;

		std::unique_ptr<UploadBuffer<PassConstants>> passCB = nullptr;
		std::unique_ptr<UploadBuffer<ObjectConstants>> meshObjectCB[DX12_RENDER_TYPE_COUNT];
		std::unique_ptr<UploadBuffer<OnlyTexObjectConstants>> pointCB = nullptr;
		std::unique_ptr<UploadBuffer<AniBoneMat>> aniBoneMatBuffer = nullptr;
	};

	struct DynamicBuffer
	{
		DynamicBuffer(ID3D12Device* device, unsigned int _renderID, unsigned int _numVertex, unsigned int _numIndex)
		{
			dynamicVertexBuffer = std::make_unique<UploadBuffer<Vertex>>(device, _numVertex, false);
			dynamicIndexBuffer = std::make_unique<UploadBuffer<UINT>>(device, _numIndex, false);
			dynamicBufferInfo = std::make_unique<DynamicBufferInfo>(_renderID, _numVertex, _numIndex, 
				dynamicVertexBuffer->GetMappedData(), dynamicIndexBuffer->GetMappedData());
		}

		std::unique_ptr<DynamicBufferInfo>			dynamicBufferInfo;
		std::unique_ptr<UploadBuffer<Vertex>>		dynamicVertexBuffer;
		std::unique_ptr<UploadBuffer<unsigned int>>	dynamicIndexBuffer;
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
	virtual std::unordered_map<std::string, MeshObject>*								GetMeshDataMap() override { return &m_Meshs; }
	virtual std::unordered_map<std::string, Ani::SkinnedData>*							GetSkinnedDataMap() override {return &m_SkinnedDatas;}
	virtual std::unordered_map<std::string, std::unique_ptr<AniTree::AnimationTree>>*	GetAnimationTreeMap() override {return &m_AniTreeDatas;}

	virtual bool	CreateMesh(const std::string& meshName, MeshObject& meshinfo, const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices) override;
	virtual bool	CreateMaterials(const std::vector<std::string>& materialNames, const std::vector<Material>& materials) override;
	virtual bool	EditMesh(const std::string& meshName, const std::vector<Vertex>& vertices) override;
	virtual bool	EditMaterial(const std::string& materialName, const Material& material, const std::string& textureName = "") override;

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
	void						FlushCommandQueue();
	void						CreateCommandObject();
	void						CreateRtvAndDsvDescriptorHeaps();
	void						CreateSwapChain();

	ID3D12Resource*				CurrentBackBuffer() const;
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const;

	ComPtr<ID3DBlob>			CompileShader(	const std::wstring& filename,
												const D3D_SHADER_MACRO* defines,
												const std::string& entrypoint,
												const std::string& target);
private: // Base object Builds
	void BuildFrameResources();
	void BuildRootSignature();
	void BuildShadersAndInputLayout();
	void BuildPSOs();

private: // Used in frame.
	void UpdateMainPassCB();
	void UpdateObjectCB();
	void UpdateAniBoneBuffer();

private:
	void DrawObject(DX12_RENDER_TYPE type);
	void DrawNormalMesh();
	void DrawDynamicMehs();
	void DrawSkinnedMesh();
	void DrawPointObjects();

private:
	D3D12_VIEWPORT						m_ScreenViewport;
	D3D12_RECT							m_ScissorRect;
	std::wstring						m_MainWndCaption = L"DX12";
	D3D_DRIVER_TYPE						m_D3dDriverType = D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_UNKNOWN;

	ComPtr<IDXGIFactory4>				m_DxgiFactory;
	ComPtr<IDXGISwapChain>				m_SwapChain;
	DXGI_FORMAT							m_BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM; // 0~1 
	DXGI_FORMAT							m_DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	ComPtr<ID3D12Device>				m_D3dDevice;

	ComPtr<ID3D12Fence>					m_Fence;
	UINT64								m_CurrentFence = 0;

	ComPtr<ID3D12CommandQueue>			m_CommandQueue;
	ComPtr<ID3D12CommandAllocator>		m_DirectCmdListAlloc;
	ComPtr<ID3D12GraphicsCommandList>	m_CommandList;

	static const int					SwapChainBufferCount = 2;
	int									m_CurrBackBuffer = 0;
	ComPtr<ID3D12Resource>				m_SwapChainBuffer[SwapChainBufferCount];
	ComPtr<ID3D12Resource>				m_DepthStencilBuffer;

	ComPtr<ID3D12DescriptorHeap>		m_RTVHeap;
	ComPtr<ID3D12DescriptorHeap>		m_DSVHeap;
	UINT								m_RTVDescriptorSize = 0;
	UINT								m_DSVDescriptorSize = 0;
	UINT								m_CBV_SRV_UAV_DescriptorSize = 0;

	bool								m_4xMsaaState = false;
	UINT								m_4xmsaaQuality = 0;

	physx::PxVec3						m_RayOrigin;
	physx::PxVec3						m_Ray;

private:
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>>	m_PSOs;
	std::vector<D3D12_INPUT_ELEMENT_DESC>							m_InputLayout[DX12_RENDER_TYPE_COUNT];
	std::unordered_map<std::string, ComPtr<ID3DBlob>>				m_Shaders;
	ComPtr<ID3D12RootSignature>										m_T1RootSignature;
	ComPtr<ID3D12RootSignature>										m_P1RootSignature;
	
	PassConstants													m_MainPassCB;
	std::unique_ptr<FrameResource>									m_FrameResource;
	std::unique_ptr<DX12FontManager>								m_FontManager;

	std::unique_ptr<cTextureBuffer>												m_TextureBuffer;
	std::unique_ptr<cIndexManagementBuffer<Material>>							m_Materials;
	std::unordered_map<std::string, MeshObject>									m_Meshs;
	std::unordered_map<std::string, Ani::SkinnedData>							m_SkinnedDatas;
	std::unordered_map<std::string, std::unique_ptr<AniTree::AnimationTree>>	m_AniTreeDatas;


private:
	std::vector<ObjectConstants>					m_RenderObjects[DX12_RENDER_TYPE_COUNT];
	std::vector<const SubmeshData*>					m_RenderObjectsSubmesh[DX12_RENDER_TYPE_COUNT];
	unsigned int									m_NumRenderPointObjects;

private:
	std::unique_ptr<cDefaultBuffer<Vertex>>			m_VertexBuffer;
	std::unique_ptr<cDefaultBuffer<UINT>>			m_IndexBuffer;

	std::unique_ptr<cDefaultBuffer<SkinnedVertex>>	m_SkinnedVertexBuffer;
	std::unique_ptr<cDefaultBuffer<UINT>>			m_SkinnedIndexBuffer;

	std::unique_ptr<UploadBuffer<B_P_Vertex>>		m_Box_Plane_Vertices;

	std::vector<std::unique_ptr<DynamicBuffer>>		m_DynamicBuffers;
};
