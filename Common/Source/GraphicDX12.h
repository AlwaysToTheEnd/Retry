#pragma once
#include "IGraphicDevice.h"

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

#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

using Microsoft::WRL::ComPtr;
using namespace DirectX;

template <typename T>
class UploadBuffer
{
public:
	UploadBuffer(ID3D12Device* device, UINT elementCount, bool isConstantBuffer)
	{
		m_ElementByteSize = sizeof(T);
		m_IsConstantBuffer = isConstantBuffer;

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
		memcpy(&m_MappedData[elementIndex * m_ElementByteSize], &data, sizeof(T));
	}

private:
	ComPtr<ID3D12Resource> m_UploadBuffer;
	BYTE* m_MappedData = nullptr;

	UINT m_ElementByteSize = 0;
	bool m_IsConstantBuffer = false;
};

struct Material
{
	std::string	name;
	int			matCBIndex = -1;

	XMFLOAT4	diffuseAlbedo = { 1,1,1,1 };
	XMFLOAT3	fresnel0 = { 0.01f,0.01f,0.01f };
	float		roughness = 0.25f;
	CGH::MAT16	matTransform;
	UINT		diffuseMapIndex = 0;
};

struct MaterialConstants
{
	XMFLOAT4	diffuseAlbedo = { 1,1,1,1 };
	XMFLOAT3	fresnel0 = { 0.01f,0.01f,0.01f };
	float		roughness = 0.25f;

	CGH::MAT16	matTransform;
	UINT		diffuseMapIndex = 0;
	UINT		MaterialPad0;
	UINT		MaterialPad1;
	UINT		MaterialPad2;
};

struct Light
{
	XMFLOAT3 strength = { 0.5f,0.5f,0.5f };
	float falloffStart = 1.0f;
	XMFLOAT3 direction = { 0,-1.0f,0 };
	float falloffEnd = 10.0f;
	XMFLOAT3 position = { 0,0,0 };
	float spotPower = 64.0f;
};

struct PassConstants
{
	CGH::MAT16 view;
	CGH::MAT16 invView;
	CGH::MAT16 proj;
	CGH::MAT16 invProj;
	CGH::MAT16 viewProj;
	CGH::MAT16 invViewProj;
	CGH::MAT16 rightViewProj;
	CGH::MAT16 shadowMapMatrix;
	XMFLOAT3 eyePosW = { 0.0f, 0.0f, 0.0f };
	float cbPerObjectPad1 = 0.0f;
	XMFLOAT2 renderTargetSize = { 0.0f, 0.0f };
	XMFLOAT2 invRenderTargetSize = { 0.0f, 0.0f };
	
	XMFLOAT4 ambientLight = { 0.0f, 0.0f, 0.0f, 1.0f };

	Light Lights[16];
};

struct FrameResource
{
	FrameResource(ID3D12Device* device, UINT passCount, UINT materialCount)
	{
		ThrowIfFailed(device->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(cmdListAlloc.GetAddressOf())));

		passCB = std::make_unique<UploadBuffer<PassConstants>>(device, passCount, true);
		materialBuffer = std::make_unique<UploadBuffer<MaterialConstants>>(device, materialCount, false);
	}

	FrameResource(const FrameResource& rhs) = delete;
	FrameResource& operator=(const FrameResource& rhs) = delete;

	ComPtr<ID3D12CommandAllocator> cmdListAlloc;

	std::unique_ptr<UploadBuffer<PassConstants>> passCB = nullptr;
	std::unique_ptr<UploadBuffer<MaterialConstants>> materialBuffer = nullptr;
	//std::unique_ptr<UploadBuffer<ObjectConstants>> ObjectCB = nullptr;
};

class GraphicDX12 final : public IGraphicDevice
{
public:
	GraphicDX12();
	virtual ~GraphicDX12() override;

	virtual void Update() override;
	virtual void Draw() override;
	virtual bool Init(HWND hWnd) override;
	virtual void OnResize() override;
	virtual void* GetDevicePtr() override { return m_D3dDevice.Get(); }
	virtual std::shared_ptr<IComponent> CreateComponent(PxTransform& trans) override;

public: // Used Functions
	virtual void SetCamera(cCamera* camera) { m_currCamera = camera; }

private: // Device Base Functions
	void CreateCommandObject();
	void CreateRtvAndDsvDescriptorHeaps();
	void CreateSwapChain();

	ID3D12Resource* CurrentBackBuffer() const;
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const;
	void FlushCommandQueue();

private: // Object Base Builds
	void BuildFrameResources();
	void BuildTextures();
	void BuildMaterials();
	void BuildRootSignature();
	void BuildShadersAndInputLayout();
	void BuildGeometry();
	void BuildPSOs();
	void BuildRenderItem();

private:
	void UpdateMainPassCB();
	void UpdateObjects();

private:
	ComPtr<ID3DBlob> CompileShader(	const std::wstring& filename,
									const D3D_SHADER_MACRO* defines,
									const std::string& entrypoint,
									const std::string& target);
private:
	D3D12_VIEWPORT	m_ScreenViewport;
	D3D12_RECT		m_ScissorRect;
	std::wstring	m_MainWndCaption = L"DX12";
	D3D_DRIVER_TYPE	m_D3dDriverType = D3D_DRIVER_TYPE_HARDWARE;

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
	std::unordered_map<std::string, std::unique_ptr<Material>>		m_Materials;
	std::vector<D3D12_INPUT_ELEMENT_DESC>							m_NTVertexInputLayout;
	std::unordered_map<std::string, ComPtr<ID3DBlob>>				m_Shaders;
	ComPtr<ID3D12RootSignature>										m_RootSignature = nullptr;

	std::unique_ptr<FrameResource>	m_FrameResource;
	PassConstants					m_MainPassCB;
};

