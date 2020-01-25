#include <d3dx12.h>
#include "DX12SwapChain.h"
#include "d3dUtil.h"

using Microsoft::WRL::ComPtr;

DX12SwapChain::DX12SwapChain()
	: m_Device(nullptr)
	, m_ColorBufferFormat(DXGI_FORMAT_R8G8B8A8_UNORM)
	, m_DepthStencilFormat(DXGI_FORMAT_D24_UNORM_S8_UINT)
	, m_NumSwapBuffer(2)
	, m_CurrBackBuffer(0)
	, m_RTVDescriptorSize(0)
	, m_DSVDescriptorSize(0)
{

}

DX12SwapChain::~DX12SwapChain()
{

}

void DX12SwapChain::CreateDXGIFactory(ID3D12Device** device)
{
	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(m_DxgiFactory.GetAddressOf())));

	if (*device == nullptr)
	{
		ComPtr<IDXGIAdapter> pWarpAdapter;
		ThrowIfFailed(m_DxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(pWarpAdapter.GetAddressOf())));

		ThrowIfFailed(D3D12CreateDevice(pWarpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(device)));
	}

	m_Device = *device;

	m_RTVDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_DSVDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	m_SRVDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void DX12SwapChain::CreateSwapChain(HWND handle, ID3D12CommandQueue* queue,
									DXGI_FORMAT renderTarget, DXGI_FORMAT depthStencil,
									unsigned int x, unsigned int y, unsigned int numSwapBuffer)
{
	m_NumSwapBuffer = numSwapBuffer;
	m_ColorBufferFormat = renderTarget;
	m_DepthStencilFormat = depthStencil;
	m_NormalBufferFormat = DXGI_FORMAT_R11G11B10_FLOAT;
	m_SpecPowBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = x;
	sd.BufferDesc.Height = y;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = m_ColorBufferFormat;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = numSwapBuffer;
	sd.OutputWindow = handle;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	ThrowIfFailed(m_DxgiFactory->CreateSwapChain(queue, &sd, m_SwapChain.GetAddressOf()));

	m_Resources.resize(GBUFFER_RESOURCE_COLORS + m_NumSwapBuffer);

	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	rtvHeapDesc.NumDescriptors = m_Resources.size() - 1;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	srvHeapDesc.NodeMask = 0;
	srvHeapDesc.NumDescriptors = GBUFFER_RESOURCE_COUNT * m_NumSwapBuffer;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	ThrowIfFailed(m_Device->CreateDescriptorHeap(&rtvHeapDesc,
		IID_PPV_ARGS(m_RTVHeap.GetAddressOf())));
	ThrowIfFailed(m_Device->CreateDescriptorHeap(&dsvHeapDesc,
		IID_PPV_ARGS(m_DSVHeap.GetAddressOf())));
	ThrowIfFailed(m_Device->CreateDescriptorHeap(&srvHeapDesc,
		IID_PPV_ARGS(m_SRVHeap.GetAddressOf())));
}

void DX12SwapChain::ReSize(ID3D12GraphicsCommandList* cmd, unsigned int x, unsigned int y)
{
	assert(m_SwapChain);

	for (int i = 0; i < m_Resources.size(); i++)
	{
		m_Resources[i] = nullptr;
	}

	CreateResources(x, y);
	CreateResourceViews();

	cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_Resources[GBUFFER_RESOURCE_DS].Get(),
			D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE));
}

void DX12SwapChain::RenderBegin(ID3D12GraphicsCommandList* cmd, const float clearColor[4])
{
	auto depthStencil = m_DSVHeap->GetCPUDescriptorHandleForHeapStart();
	auto base = CurrentBackBufferView();
	auto normal = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_RTVHeap->GetCPUDescriptorHandleForHeapStart(), m_NumSwapBuffer, m_RTVDescriptorSize);
	auto specular = normal;
	specular.Offset(1, m_RTVDescriptorSize);

	D3D12_CPU_DESCRIPTOR_HANDLE renderTargetHandles[] =
	{
		base,
		normal,
		specular
	};

	cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_Resources[GBUFFER_RESOURCE_COLORS + m_CurrBackBuffer].Get(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	cmd->OMSetRenderTargets(_countof(renderTargetHandles), renderTargetHandles, false, &depthStencil);
	cmd->ClearRenderTargetView(base, clearColor, 0, nullptr);
	cmd->ClearRenderTargetView(normal, zero, 0, nullptr);
	cmd->ClearRenderTargetView(specular, zero, 0, nullptr);
	cmd->ClearDepthStencilView(depthStencil, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
}

void DX12SwapChain::RenderEnd(ID3D12GraphicsCommandList* cmd)
{
	cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_Resources[GBUFFER_RESOURCE_COLORS + m_CurrBackBuffer].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
}

void DX12SwapChain::Presnet()
{
	ThrowIfFailed(m_SwapChain->Present(0, 0));
	m_CurrBackBuffer = (m_CurrBackBuffer + 1) % m_NumSwapBuffer;
}

void DX12SwapChain::GetRenderTargetFormats(std::vector<DXGI_FORMAT>& out)
{
	out.clear();
	out.push_back(m_ColorBufferFormat);
	out.push_back(DXGI_FORMAT_R11G11B10_FLOAT);
	out.push_back(DXGI_FORMAT_R8G8B8A8_UNORM);
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12SwapChain::CurrentBackBufferView() const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_RTVHeap->GetCPUDescriptorHandleForHeapStart(),
		m_CurrBackBuffer, m_RTVDescriptorSize);
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12SwapChain::DepthStencilView() const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_DSVHeap->GetCPUDescriptorHandleForHeapStart());
}

D3D12_GPU_DESCRIPTOR_HANDLE DX12SwapChain::CurrSRVsGPU() const
{
	auto result = m_SRVHeap->GetGPUDescriptorHandleForHeapStart();
	result.ptr += (m_CurrBackBuffer * GBUFFER_RESOURCE_COUNT * m_SRVDescriptorSize);
	return result;
}

void DX12SwapChain::CreateResources(unsigned int x, unsigned int y)
{
	ThrowIfFailed(m_SwapChain->ResizeBuffers(m_NumSwapBuffer,
		x, y, m_ColorBufferFormat, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));
	m_CurrBackBuffer = 0;

	D3D12_RESOURCE_DESC normalDesc = {};
	normalDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	normalDesc.Format = m_NormalBufferFormat;
	normalDesc.DepthOrArraySize = 1;
	normalDesc.MipLevels = 1;
	normalDesc.Width = x;
	normalDesc.Height = y;
	normalDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	normalDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	normalDesc.SampleDesc.Count = 1;
	normalDesc.SampleDesc.Quality = 0;

	D3D12_RESOURCE_DESC specularDesc = {};
	specularDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	specularDesc.Format = m_SpecPowBufferFormat;
	specularDesc.DepthOrArraySize = 1;
	specularDesc.MipLevels = 1;
	specularDesc.Width = x;
	specularDesc.Height = y;
	specularDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	specularDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	specularDesc.SampleDesc.Count = 1;
	specularDesc.SampleDesc.Quality = 0;

	D3D12_RESOURCE_DESC dsDesc;
	dsDesc.Alignment = 0;
	dsDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	dsDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	dsDesc.DepthOrArraySize = 1;
	dsDesc.MipLevels = 1;
	dsDesc.Width = x;
	dsDesc.Height = y;
	dsDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	dsDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	dsDesc.SampleDesc.Count = 1;
	dsDesc.SampleDesc.Quality = 0;

	for (int i = 0; i < m_NumSwapBuffer; i++)
	{
		ThrowIfFailed(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(m_Resources[GBUFFER_RESOURCE_COLORS + i].GetAddressOf())));
	}

	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = normalDesc.Format;

	ThrowIfFailed(m_Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &normalDesc,
		D3D12_RESOURCE_STATE_RENDER_TARGET, &clearValue, IID_PPV_ARGS(m_Resources[GBUFFER_RESOURCE_NORMAL].GetAddressOf())));

	clearValue.Format = specularDesc.Format;

	ThrowIfFailed(m_Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &specularDesc,
		D3D12_RESOURCE_STATE_RENDER_TARGET, &clearValue, IID_PPV_ARGS(m_Resources[GBUFFER_RESOURCE_SPECPOWER].GetAddressOf())));
	
	clearValue.Format = m_DepthStencilFormat;
	clearValue.DepthStencil.Depth = 1.0f;
	clearValue.DepthStencil.Stencil = 0;

	ThrowIfFailed(m_Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &dsDesc,
		D3D12_RESOURCE_STATE_COMMON, &clearValue, IID_PPV_ARGS(m_Resources[GBUFFER_RESOURCE_DS].GetAddressOf())));
}


void DX12SwapChain::CreateResourceViews()
{
	//Fill rtvHeap.
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RTVHeap->GetCPUDescriptorHandleForHeapStart());

	for (size_t i = 0; i < m_NumSwapBuffer; i++)
	{
		m_Device->CreateRenderTargetView(m_Resources[GBUFFER_RESOURCE_COLORS + i].Get(), nullptr, rtvHandle);
		rtvHandle.Offset(1, m_RTVDescriptorSize);
	}

	m_Device->CreateRenderTargetView(m_Resources[GBUFFER_RESOURCE_NORMAL].Get(), nullptr, rtvHandle);
	rtvHandle.Offset(1, m_RTVDescriptorSize);
	m_Device->CreateRenderTargetView(m_Resources[GBUFFER_RESOURCE_SPECPOWER].Get(), nullptr, rtvHandle);

	//Fill dsvHeap.
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Format = m_DepthStencilFormat;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;

	m_Device->CreateDepthStencilView(m_Resources[GBUFFER_RESOURCE_DS].Get(), &dsvDesc,
		m_DSVHeap->GetCPUDescriptorHandleForHeapStart());

	//Fill srvHeap
	CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(m_SRVHeap->GetCPUDescriptorHandleForHeapStart());

	for (size_t i = 0; i < m_NumSwapBuffer; i++)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;	
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		
		m_Device->CreateShaderResourceView(m_Resources[GBUFFER_RESOURCE_DS].Get(), &srvDesc, srvHandle);
		srvHandle.Offset(1, m_SRVDescriptorSize);

		srvDesc.Format = m_NormalBufferFormat;
		m_Device->CreateShaderResourceView(m_Resources[GBUFFER_RESOURCE_NORMAL].Get(), nullptr, srvHandle);
		srvHandle.Offset(1, m_SRVDescriptorSize);

		srvDesc.Format = m_SpecPowBufferFormat;
		m_Device->CreateShaderResourceView(m_Resources[GBUFFER_RESOURCE_SPECPOWER].Get(), nullptr, srvHandle);
		srvHandle.Offset(1, m_SRVDescriptorSize);

		srvDesc.Format = m_ColorBufferFormat;
		m_Device->CreateShaderResourceView(m_Resources[GBUFFER_RESOURCE_COLORS + i].Get(), nullptr, srvHandle);
		srvHandle.Offset(1, m_SRVDescriptorSize);
	}
}
