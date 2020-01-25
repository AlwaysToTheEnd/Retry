#include <d3dx12.h>
#include "DX12SwapChain.h"
#include "d3dUtil.h"

using Microsoft::WRL::ComPtr;

DX12SwapChain::DX12SwapChain()
	: m_Device(nullptr)
	, m_BackBufferFormat(DXGI_FORMAT_R8G8B8A8_UNORM)
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
}

void DX12SwapChain::CreateSwapChain(HWND handle, ID3D12CommandQueue* queue,
									DXGI_FORMAT renderTarget, DXGI_FORMAT depthStencil,
									unsigned int x, unsigned int y, unsigned int numSwapBuffer)
{
	m_NumSwapBuffer = numSwapBuffer;
	m_BackBufferFormat = renderTarget;
	m_DepthStencilFormat = depthStencil;

	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = x;
	sd.BufferDesc.Height = y;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = m_BackBufferFormat;
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

	m_RenderTargetBuffer.resize(m_NumSwapBuffer + 2);

	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	rtvHeapDesc.NumDescriptors = m_NumSwapBuffer + 2;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

	ThrowIfFailed(m_Device->CreateDescriptorHeap(&rtvHeapDesc,
		IID_PPV_ARGS(m_RTVHeap.GetAddressOf())));
	ThrowIfFailed(m_Device->CreateDescriptorHeap(&dsvHeapDesc,
		IID_PPV_ARGS(m_DSVHeap.GetAddressOf())));
}

void DX12SwapChain::ReSize(ID3D12GraphicsCommandList* cmd, unsigned int x, unsigned int y)
{
	assert(m_SwapChain);

	for (int i = 0; i < m_RenderTargetBuffer.size(); i++)
	{
		m_RenderTargetBuffer[i] = nullptr;
	}

	m_DepthStencilBuffer = nullptr;

	ThrowIfFailed(m_SwapChain->ResizeBuffers(m_NumSwapBuffer,
		x, y, m_BackBufferFormat, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));
	m_CurrBackBuffer = 0;

	D3D12_RESOURCE_DESC normalDesc = {};
	normalDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	normalDesc.Format = DXGI_FORMAT_R11G11B10_FLOAT;
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
	specularDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	specularDesc.DepthOrArraySize = 1;
	specularDesc.MipLevels = 1;
	specularDesc.Width = x;
	specularDesc.Height = y;
	specularDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	specularDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	specularDesc.SampleDesc.Count = 1;
	specularDesc.SampleDesc.Quality = 0;

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RTVHeap->GetCPUDescriptorHandleForHeapStart());

	for (int i = 0; i < m_NumSwapBuffer; i++)
	{
		ThrowIfFailed(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(m_RenderTargetBuffer[i].GetAddressOf())));

		m_Device->CreateRenderTargetView(m_RenderTargetBuffer[i].Get(), nullptr, rtvHandle);

		rtvHandle.Offset(1, m_RTVDescriptorSize);
	}
	
	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = normalDesc.Format;

	ThrowIfFailed(m_Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &normalDesc,
		D3D12_RESOURCE_STATE_RENDER_TARGET, &clearValue, IID_PPV_ARGS(m_RenderTargetBuffer[m_NumSwapBuffer].GetAddressOf())));

	clearValue.Format = specularDesc.Format;

	ThrowIfFailed(m_Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &specularDesc,
		D3D12_RESOURCE_STATE_RENDER_TARGET, &clearValue, IID_PPV_ARGS(m_RenderTargetBuffer[m_NumSwapBuffer + 1].GetAddressOf())));

	m_Device->CreateRenderTargetView(m_RenderTargetBuffer[m_NumSwapBuffer].Get(), nullptr,
		rtvHandle);

	rtvHandle.Offset(1, m_RTVDescriptorSize);
	
	m_Device->CreateRenderTargetView(m_RenderTargetBuffer[m_NumSwapBuffer + 1].Get(), nullptr,
		rtvHandle);

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

	clearValue.Format = m_DepthStencilFormat;
	clearValue.DepthStencil.Depth = 1.0f;
	clearValue.DepthStencil.Stencil = 0;

	ThrowIfFailed(m_Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &dsDesc,
		D3D12_RESOURCE_STATE_COMMON, &clearValue, IID_PPV_ARGS(m_DepthStencilBuffer.GetAddressOf())));

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Format = m_DepthStencilFormat;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;

	m_Device->CreateDepthStencilView(m_DepthStencilBuffer.Get(), &dsvDesc,
		m_DSVHeap->GetCPUDescriptorHandleForHeapStart());

	cmd->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(m_DepthStencilBuffer.Get(),
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

	cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_RenderTargetBuffer[m_CurrBackBuffer].Get(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	cmd->OMSetRenderTargets(_countof(renderTargetHandles), renderTargetHandles, false, &depthStencil);
	cmd->ClearRenderTargetView(base, clearColor, 0, nullptr);
	cmd->ClearRenderTargetView(normal, zero, 0, nullptr);
	cmd->ClearRenderTargetView(specular, zero, 0, nullptr);
	cmd->ClearDepthStencilView(depthStencil, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
}

void DX12SwapChain::RenderEnd(ID3D12GraphicsCommandList* cmd)
{
	cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_RenderTargetBuffer[m_CurrBackBuffer].Get(),
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
	out.push_back(m_BackBufferFormat);
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
