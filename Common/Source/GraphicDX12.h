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
#include "IGraphicDevice.h"
#include "Vertex.h"

#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

class cTextureBuffer;

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
	std::unique_ptr<UploadBuffer<ObjectConstants>> ObjectCB = nullptr;
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
	virtual std::unique_ptr<IComponent> CreateComponent(PxTransform& trans) override;

public: // Used Functions
	virtual void SetCamera(cCamera* camera) { m_currCamera = camera; }
	virtual void AddMesh(UINT numSubResource, SubmeshData subResources[]) override;

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
	std::unique_ptr<cTextureBuffer>									m_TextureBuffer;

	std::unique_ptr<FrameResource>	m_FrameResource;
	PassConstants					m_MainPassCB;

private: // Codes below are used only Testing.
	ComPtr<ID3D12Resource>			m_VertexBuffer;
	ComPtr<ID3D12Resource>			m_VertexUploadBuffer;
};

