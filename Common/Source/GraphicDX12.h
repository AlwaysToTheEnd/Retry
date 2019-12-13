#pragma once
#include <wrl.h>
#include <dxgi1_4.h>
#include <d3d12.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>
#include <algorithm>
#include "d3dx12.h"
#include "d3dx12Residency.h"
#include "DX12RenderClasses.h"
#include "cIndexManagementBuffer.h"
#include "IGraphicDevice.h"
#include "AnimationStructs.h"

#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

#include "XFileParser.h"	

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

	UINT GetElementByteSize() const { return m_ElementByteSize; }

private:
	ComPtr<ID3D12Resource> m_UploadBuffer;
	BYTE* m_MappedData = nullptr;

	UINT	m_NumElement = 0; 
	UINT64	m_ElementByteSize = 0;
	bool	m_IsConstantBuffer = false;
};

struct FrameResource
{
	FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount,UINT aniBoneSetNum)
	{
		ThrowIfFailed(device->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(cmdListAlloc.GetAddressOf())));

		passCB = std::make_unique<UploadBuffer<PassConstants>>(device, passCount, true);
		objectCB = std::make_unique<UploadBuffer<ObjectConstants>>(device, objectCount, true);
		aniBoneMatBuffer = std::make_unique<UploadBuffer<AniBoneMat>>(device, aniBoneSetNum, true);
	}

	FrameResource(const FrameResource& rhs) = delete;
	FrameResource& operator=(const FrameResource& rhs) = delete;

	ComPtr<ID3D12CommandAllocator> cmdListAlloc;
	
	std::unique_ptr<UploadBuffer<PassConstants>> passCB = nullptr;
	std::unique_ptr<UploadBuffer<ObjectConstants>> objectCB = nullptr;
	std::unique_ptr<UploadBuffer<AniBoneMat>> aniBoneMatBuffer = nullptr;
};

class GraphicDX12 final : public IGraphicDevice
{
	enum
	{
		MATERIAL_BUFFER,
		PASS_CB,
		OBJECT_CB,
		TEXTURE_TABLE,
		ANIBONE_BUFFER,
		ROOT_COUNT
	};

public:
	GraphicDX12();
	virtual ~GraphicDX12() override;

	virtual void Update() override;
	virtual void Draw() override;
	virtual void ReservedWorksClear() override;
	virtual bool Init(HWND hWnd) override;
	virtual void OnResize() override;
	virtual void* GetDevicePtr() override { return m_D3dDevice.Get(); }
	virtual std::unique_ptr<IComponent> CreateComponent(COMPONENTTYPE type, GameObject& gameObject) override;
	
public: // Used Functions
	virtual void SetCamera(cCamera* camera) { m_currCamera = camera; }

private: // Only Used by FuncPtr
	virtual void ComponentDeleteManaging(COMPONENTTYPE type, int id) override;

private: // Used Function by ReadyWorks 
	virtual void LoadTextureFromFolder(const std::vector<std::string>& targetTextureFolders) override;
	virtual void LoadMeshAndMaterialFromFolder(const std::vector<std::string>& targetMeshFolders) override;
	virtual void ReadyWorksEnd() override;

private: // Device Base Functions
	void FlushCommandQueue();
	void CreateCommandObject();
	void CreateRtvAndDsvDescriptorHeaps();
	void CreateSwapChain();

	ID3D12Resource* CurrentBackBuffer() const;
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const;

private: // Base object Builds
	void BuildFrameResources();
	void BuildRootSignature();
	void BuildShadersAndInputLayout();
	void BuildPSOs();

private:
	void UpdateMainPassCB();
	void UpdateObjectCB();
	void UpdateAniBoneBuffer();

private:
	void DrawObjects();

private:
	ComPtr<ID3DBlob> CompileShader(	const std::wstring& filename,
									const D3D_SHADER_MACRO* defines,
									const std::string& entrypoint,
									const std::string& target);
private:
	D3D12_VIEWPORT	m_ScreenViewport;
	D3D12_RECT		m_ScissorRect;
	std::wstring	m_MainWndCaption = L"DX12";
	D3D_DRIVER_TYPE	m_D3dDriverType = D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_UNKNOWN;

	ComPtr<IDXGIFactory4>	m_DxgiFactory;
	ComPtr<IDXGISwapChain>	m_SwapChain;
	DXGI_FORMAT				m_BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM; // 0~1 
	DXGI_FORMAT				m_DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	ComPtr<ID3D12Device>	m_D3dDevice;

	ComPtr<ID3D12Fence>		m_Fence;
	UINT64					m_CurrentFence = 0;

	ComPtr<ID3D12CommandQueue>		m_CommandQueue;
	ComPtr<ID3D12CommandAllocator>	m_DirectCmdListAlloc;
	ComPtr<ID3D12GraphicsCommandList> m_CommandList;

	static const int				SwapChainBufferCount = 2;
	int								m_CurrBackBuffer = 0;
	ComPtr<ID3D12Resource>			m_SwapChainBuffer[SwapChainBufferCount];
	ComPtr<ID3D12Resource>			m_DepthStencilBuffer;

	ComPtr<ID3D12DescriptorHeap>	m_RTVHeap;
	ComPtr<ID3D12DescriptorHeap>	m_DSVHeap;
	UINT							m_RTVDescriptorSize = 0;
	UINT							m_DSVDescriptorSize = 0;
	UINT							m_CBV_SRV_UAV_DescriptorSize = 0;

	bool							m_4xMsaaState = false;
	UINT							m_4xmsaaQuality = 0;

	cCamera*						m_currCamera = nullptr;

private:
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>>	m_PSOs;
	std::vector<D3D12_INPUT_ELEMENT_DESC>							m_NTVertexInputLayout;
	std::unordered_map<std::string, ComPtr<ID3DBlob>>				m_Shaders;
	ComPtr<ID3D12RootSignature>										m_RootSignature = nullptr;
	
	PassConstants													m_MainPassCB;
	std::unique_ptr<cTextureBuffer>									m_TextureBuffer;
	std::unique_ptr<cIndexManagementBuffer<Material>>				m_Materials;
	std::unordered_map<std::string, MeshObject>						m_Meshs;
	std::unordered_map<std::string, Ani::SkinnedData>				m_SkinnedDatas;
	std::unique_ptr<FrameResource>									m_FrameResource;

private:
	std::vector<AniBoneMat>							m_ReservedAniBones;
	std::vector<RenderInfo>							m_ReservedRenders;

	std::vector<ObjectConstants>					m_RenderObjects;
	std::vector<const SubmeshData*>					m_RenderObjectsSubmesh;

private:
	std::unique_ptr<cDefaultBuffer<Vertex>>			m_VertexBuffer;
	std::unique_ptr<cDefaultBuffer<UINT>>			m_IndexBuffer;
	std::unique_ptr<cDefaultBuffer<SkinnedVertex>>	m_SkinnedVertexBuffer;
	std::unique_ptr<cDefaultBuffer<UINT>>			m_SkinnedIndexBuffer;
};
