#include "GraphicDX12.h"
#include "cCamera.h"
#include "cTextureBuffer.h"
#include <functional>
#include "GraphicComponent.h"

using namespace DirectX;
using namespace std;

GraphicDX12::GraphicDX12()
{
}

GraphicDX12::~GraphicDX12()
{
}

bool GraphicDX12::Init(HWND hWnd)
{
	m_MainWndHandle = hWnd;

	//����� ����. ���־� ��Ʃ��� ���â�� ����� ������ ����ش�.
#if defined(DEBUG)||defined(_DEBUG)
	{
		ComPtr<ID3D12Debug> debugController;
		ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(debugController.GetAddressOf())));
		debugController->EnableDebugLayer();
	}
#endif

	HRESULT hr = S_OK;

	//���� ü���� ����� ���ؼ� DXGI Factory�� �ʿ���.
	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(m_DxgiFactory.GetAddressOf())));

	//D12 Device ����
	hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(m_D3dDevice.GetAddressOf()));

	//feature Level�� �������� ���ϴ� �ϵ����(ex �����)�� ����Ʈ������(cpu)�� WarpAdapter�� �����
	if (FAILED(hr))
	{
		ComPtr<IDXGIAdapter> pWarpAdapter;
		ThrowIfFailed(m_DxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(pWarpAdapter.GetAddressOf())));

		ThrowIfFailed(D3D12CreateDevice(pWarpAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(m_D3dDevice.GetAddressOf())));
	}

	//cpu�� gpu �۾�ť ����ȭ�� ���� �潺 ����
	ThrowIfFailed(m_D3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(m_Fence.GetAddressOf())));

	//�� �ڿ��� ������ ũ�⸦ �̸� �����ص�. �������⿡�� �Ű������� ���� ����
	m_RTVDescriptorSize = m_D3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_DSVDescriptorSize = m_D3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	m_CBV_SRV_UAV_DescriptorSize = m_D3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//��Ƽ ���ø� ���� üũ
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;

	msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msQualityLevels.Format = m_BackBufferFormat;
	msQualityLevels.SampleCount = 4;
	msQualityLevels.NumQualityLevels = 0;

	ThrowIfFailed(m_D3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&msQualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS)));

	m_4xmsaaQuality = msQualityLevels.NumQualityLevels;
	assert(m_4xmsaaQuality > 0 && "Unexpected MSAA quality level.");

	// GPU�� ���� ����� ������ ���� ������Ʈ ����( commandList, queue �� )
	CreateCommandObject();

	// dx12���� ����ü���� ���� ��������
	CreateSwapChain();

	// ����ü�ο� ���� RenderTarget ������, DepthStencil �����ڰ� ��� ���� ����.
	CreateRtvAndDsvDescriptorHeaps();

	// ����Ÿ���� �� ���ҽ��� ���� Ŭ���̾�Ʈ ����� �°� �����ϰ� ����ü�ο� �����ش�
	OnResize();

	XFileParser xParser("../Common/TeraResourse/Character/poporiClass03/poporiClass03_2.X");
	m_XfileObject = xParser.GetAniObject();

#pragma region RenderObjectsBuild

	ThrowIfFailed(m_CommandList->Reset(m_DirectCmdListAlloc.Get(), nullptr));

	BuildTextures();
	BuildRootSignature();			// ****************** ���� �߿��Ѱ�. �̺κ� ������ �����ϰ� �Ѿ����
	BuildShadersAndInputLayout();	// �ٷ�9 Vertex fvf �����ϸ� ���� ������ �� ����.
	BuildPSOs();					// ������ ������ ���ο� ���� ���赵��� �����ϸ� ��.
	BuildGeometry();				//
	BuildMaterials();				//
	BuildRenderItem();				//
	BuildFrameResources();			// FrameResource�� ���ؼ� �޸��� �����͵�(ViewMatrix����)�� GPU�� ���ε��Ѵ�.

	ThrowIfFailed(m_CommandList->Close());
	ID3D12CommandList* cmdsLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	FlushCommandQueue();
#pragma endregion 

	m_VertexUploadBuffer = nullptr;
	m_IndexUploadBuffer = nullptr;
	return true;
}

void GraphicDX12::AddMesh(UINT numSubResource, SubmeshData subResources[])
{

}

void GraphicDX12::CreateCommandObject()
{
	/*	1. Ŀ�ǵ� ����Ʈ�� Allocator, Pipeline State Obejct�� ����
		2. Ŀ�ǵ� ����Ʈ�� ��ɵ��� ����
		3. Ŀ�ǵ� ť�� Ŀ�ǵ� ����Ʈ�� �迭�� Excute
		4. �׷��� ����̽����� ť���� �ϳ��� ���� ���� */

		//������ ����̽����� ������ �ϴ� ��ɵ��� ����� ť�� �����Ѵ�.
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ThrowIfFailed(m_D3dDevice->CreateCommandQueue(&queueDesc,
		IID_PPV_ARGS(m_CommandQueue.GetAddressOf())));

	ThrowIfFailed(m_D3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(m_DirectCmdListAlloc.GetAddressOf())));

	ThrowIfFailed(m_D3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_DirectCmdListAlloc.Get(), nullptr, IID_PPV_ARGS(m_CommandList.GetAddressOf())));

	m_CommandList->Close();
}

void GraphicDX12::CreateSwapChain()
{
	m_SwapChain = nullptr;

	// ����ü���� �Ӽ��� ���� ������ �� DXGIFactory�� ����.
	// DX12���� CommandQueue�� �Ű������� �־��־�� ��.
	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = m_ClientWidth;
	sd.BufferDesc.Height = m_ClientHeight;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = m_BackBufferFormat;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.SampleDesc.Count = m_4xMsaaState ? 4 : 1;
	sd.SampleDesc.Quality = m_4xMsaaState ? (m_4xmsaaQuality - 1) : 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = SwapChainBufferCount;
	sd.OutputWindow = m_MainWndHandle;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	ThrowIfFailed(m_DxgiFactory->CreateSwapChain(
		m_CommandQueue.Get(),
		&sd,
		m_SwapChain.GetAddressOf()));
}

void GraphicDX12::CreateRtvAndDsvDescriptorHeaps()
{
	// ���� Ÿ�� �����ڰ� �� �� ����.
	// ����ü���� ���ڿ� �´� ũ��� �����Ѵ� (���� ũ��= SwapChainBufferCount(2))
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	rtvHeapDesc.NumDescriptors = SwapChainBufferCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

	// ����,���Ľ� �����ڿ� �� ����.
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

	ThrowIfFailed(m_D3dDevice->CreateDescriptorHeap(&rtvHeapDesc,
		IID_PPV_ARGS(m_RTVHeap.GetAddressOf())));
	ThrowIfFailed(m_D3dDevice->CreateDescriptorHeap(&dsvHeapDesc,
		IID_PPV_ARGS(m_DSVHeap.GetAddressOf())));
}

void GraphicDX12::OnResize()
{
	assert(m_D3dDevice);
	assert(m_SwapChain);
	assert(m_DirectCmdListAlloc);
	// Command Queue�� ����ش�
	FlushCommandQueue();

	// CommandList �ʱ�ȭ
	ThrowIfFailed(m_CommandList->Reset(m_DirectCmdListAlloc.Get(), nullptr));

	// ����ü�ο� ���� ���ҽ��� ���� ������
	for (int i = 0; i < SwapChainBufferCount; i++)
	{
		m_SwapChainBuffer[i] = nullptr;
	}

	m_DepthStencilBuffer = nullptr;

	// ����ü�� ������Ʈ�� ũ�� ����
	ThrowIfFailed(m_SwapChain->ResizeBuffers(SwapChainBufferCount,
		m_ClientWidth, m_ClientHeight, m_BackBufferFormat, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));
	m_CurrBackBuffer = 0;

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RTVHeap->GetCPUDescriptorHandleForHeapStart());

	for (int i = 0; i < SwapChainBufferCount; i++)
	{
		// ����ü�� ������Ʈ�� ���� ���ҽ��� �޾ƿ´�
		ThrowIfFailed(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(m_SwapChainBuffer[i].GetAddressOf())));

		// ���� �޾ƿ� ���ҽ��� ���� �����ڸ� ���� ��, rtvHandle(���� ������) ��ġ�� ����ش�
		m_D3dDevice->CreateRenderTargetView(m_SwapChainBuffer[i].Get(), nullptr, rtvHandle);

		//rtvHandle(���� ������) ��ġ�� �����Ѵ�
		rtvHandle.Offset(1, m_RTVDescriptorSize);
	}

	// ����, ���Ľ� ���۸� �����Ѵ�.

	D3D12_RESOURCE_DESC dsvDesc;
	dsvDesc.Alignment = 0; //����
	dsvDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	dsvDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	dsvDesc.DepthOrArraySize = 1;
	dsvDesc.MipLevels = 1;
	dsvDesc.Width = m_ClientWidth;
	dsvDesc.Height = m_ClientHeight;
	dsvDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	dsvDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	dsvDesc.SampleDesc.Count = m_4xMsaaState ? 4 : 1;
	dsvDesc.SampleDesc.Quality = m_4xMsaaState ? (m_4xmsaaQuality - 1) : 0;

	D3D12_CLEAR_VALUE clearValue;
	clearValue.Format = m_DepthStencilFormat;
	clearValue.DepthStencil.Depth = 1.0f;
	clearValue.DepthStencil.Stencil = 0;

	ThrowIfFailed(m_D3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &dsvDesc,
		D3D12_RESOURCE_STATE_COMMON, &clearValue, IID_PPV_ARGS(m_DepthStencilBuffer.GetAddressOf())));

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvViewDesc;
	dsvViewDesc.Format = m_DepthStencilFormat;
	dsvViewDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvViewDesc.Texture2D.MipSlice = 0;

	//������ ����, ���Ľ� ������ �����ڸ� ���� �� ���������� ����ش�.
	m_D3dDevice->CreateDepthStencilView(m_DepthStencilBuffer.Get(), &dsvViewDesc,
		m_DSVHeap->GetCPUDescriptorHandleForHeapStart());

	//������ ����, ���Ľǹ����� ���¸� ������ ��� ������ ���·� �����Ѵ�.
	m_CommandList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(m_DepthStencilBuffer.Get(),
			D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE));

	// CommandList�� ��� ��ɵ��� ����ߴٸ� Close()
	ThrowIfFailed(m_CommandList->Close());

	// CommandQueue�� CommandList�� �迭�� �����͸� �ѱ��(����ȴ�)
	ID3D12CommandList* cmdLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

	// ��� ������ �ɶ����� ��ٸ���.
	FlushCommandQueue();

	m_ScreenViewport.TopLeftX = 0;
	m_ScreenViewport.TopLeftY = 0;
	m_ScreenViewport.Width = static_cast<float>(m_ClientWidth);
	m_ScreenViewport.Height = static_cast<float>(m_ClientHeight);
	m_ScreenViewport.MinDepth = 0.0f;
	m_ScreenViewport.MaxDepth = 1.0f;

	m_ScissorRect = { 0, 0, m_ClientWidth, m_ClientHeight };

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * XM_PI, (float)m_ClientWidth / m_ClientHeight, 1.0f, 1000.0f);
	XMStoreFloat4x4(m_ProjectionMat, P);
}

std::unique_ptr<IComponent> GraphicDX12::CreateComponent(PxTransform& trans)
{
	GraphicComponent* newComponent = new GraphicComponent(trans, this);

	return std::unique_ptr<IComponent>(static_cast<IComponent*>(newComponent));
}

void GraphicDX12::FlushCommandQueue()
{
	m_CurrentFence++;

	ThrowIfFailed(m_CommandQueue->Signal(m_Fence.Get(), m_CurrentFence));

	if (m_Fence->GetCompletedValue() < m_CurrentFence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);

		ThrowIfFailed(m_Fence->SetEventOnCompletion(m_CurrentFence, eventHandle));

		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}

void GraphicDX12::Update()
{
	if (m_currCamera)
	{
		m_ViewMatrix = *m_currCamera->GetViewMatrix();
	}

	UpdateMainPassCB();
}

void GraphicDX12::Draw()
{
	auto cmdListAlloc = m_FrameResource->cmdListAlloc.Get();

	ThrowIfFailed(cmdListAlloc->Reset());

	ThrowIfFailed(m_CommandList->Reset(cmdListAlloc,
		m_PSOs["base"].Get()));

	m_CommandList->RSSetViewports(1, &m_ScreenViewport);
	m_CommandList->RSSetScissorRects(1, &m_ScissorRect);

	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	m_CommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::Aqua, 0, nullptr);
	m_CommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	ID3D12DescriptorHeap* descriptorHeaps[] = { m_TextureBuffer->GetHeap() };
	m_CommandList->SetDescriptorHeaps(1, descriptorHeaps);
	m_CommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	m_CommandList->SetGraphicsRootSignature(m_RootSignature.Get());

	auto matBuffer = m_FrameResource->materialBuffer->Resource();
	m_CommandList->SetGraphicsRootShaderResourceView(0, matBuffer->GetGPUVirtualAddress());
	m_CommandList->SetGraphicsRootConstantBufferView(1, m_FrameResource->passCB->Resource()->GetGPUVirtualAddress());
	m_CommandList->SetGraphicsRootConstantBufferView(2, m_FrameResource->ObjectCB->Resource()->GetGPUVirtualAddress());
	m_CommandList->SetGraphicsRootDescriptorTable(3, m_TextureBuffer->GetHeap()->GetGPUDescriptorHandleForHeapStart());

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
	vertexBufferView.BufferLocation = m_VertexBuffer->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = m_VertexBufferSize;
	vertexBufferView.StrideInBytes =  sizeof(Vertex);
	
	D3D12_INDEX_BUFFER_VIEW indexBufferView = {};
	indexBufferView.BufferLocation = m_IndexBuffer->GetGPUVirtualAddress();
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	indexBufferView.SizeInBytes = m_IndexBufferSize;

	m_CommandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	m_CommandList->IASetIndexBuffer(&indexBufferView);
	m_CommandList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_CommandList->DrawIndexedInstanced(m_IndexBufferSize/sizeof(UINT), 1, 0, 0, 0);

	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
	ThrowIfFailed(m_CommandList->Close());
	ID3D12CommandList* cmdsLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	ThrowIfFailed(m_SwapChain->Present(0, 0));
	m_CurrBackBuffer = (m_CurrBackBuffer + 1) % SwapChainBufferCount;

	FlushCommandQueue();
}

ID3D12Resource* GraphicDX12::CurrentBackBuffer() const
{
	return m_SwapChainBuffer[m_CurrBackBuffer].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE GraphicDX12::CurrentBackBufferView() const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_RTVHeap->GetCPUDescriptorHandleForHeapStart(),
		m_CurrBackBuffer, m_RTVDescriptorSize);
}

D3D12_CPU_DESCRIPTOR_HANDLE GraphicDX12::DepthStencilView() const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_DSVHeap->GetCPUDescriptorHandleForHeapStart());
}

void GraphicDX12::BuildFrameResources()
{
	m_FrameResource = make_unique<FrameResource>(m_D3dDevice.Get(),
		2, (UINT)m_Materials.size());

	m_FrameResource->ObjectCB = make_unique<UploadBuffer<ObjectConstants>>(m_D3dDevice.Get(), 1, true);
}

void GraphicDX12::BuildTextures()
{
	pair<string, wstring> texturePaths[] =
	{
		{"testTexture", L"./../Common/TextureData/plane.png"},
	};

	const UINT numTexturePath = _countof(texturePaths);
	m_TextureBuffer = make_unique<cTextureBuffer>(m_D3dDevice.Get(), numTexturePath);

	m_TextureBuffer->Begin(m_D3dDevice.Get());

	for (UINT i = 0; i < numTexturePath; i++)
	{
		m_TextureBuffer->AddTexture(m_D3dDevice.Get(),
			m_CommandQueue.Get(), texturePaths->first, texturePaths->second);
	}

	m_TextureBuffer->End(m_CommandQueue.Get(), bind(&GraphicDX12::FlushCommandQueue, this));
}

void GraphicDX12::BuildMaterials()
{
	auto material = make_unique<Material>();
	material->name = "baseMaterial";
	material->matCBIndex = 1;
	material->diffuseMapIndex = 0;
	material->diffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	material->fresnel0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	material->roughness = 0.25f;

	m_Materials[material->name] = move(material);
}

void GraphicDX12::BuildRootSignature()
{
#pragma region StaticSamplers
	CD3DX12_STATIC_SAMPLER_DESC staticSamplers[7] = {};

	staticSamplers[0] = CD3DX12_STATIC_SAMPLER_DESC(
		0,
		D3D12_FILTER_MIN_MAG_MIP_POINT,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP);

	staticSamplers[1] = CD3DX12_STATIC_SAMPLER_DESC(
		1,
		D3D12_FILTER_MIN_MAG_MIP_POINT,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP);

	staticSamplers[2] = CD3DX12_STATIC_SAMPLER_DESC(
		2,
		D3D12_FILTER_MIN_MAG_MIP_LINEAR,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP);

	staticSamplers[3] = CD3DX12_STATIC_SAMPLER_DESC(
		3,
		D3D12_FILTER_MIN_MAG_MIP_LINEAR,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP);

	staticSamplers[4] = CD3DX12_STATIC_SAMPLER_DESC(
		4,
		D3D12_FILTER_ANISOTROPIC,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		0.0f,
		8);

	staticSamplers[5] = CD3DX12_STATIC_SAMPLER_DESC(
		5,
		D3D12_FILTER_ANISOTROPIC,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		0.0f,
		8);

	staticSamplers[6] = CD3DX12_STATIC_SAMPLER_DESC(
		6,
		D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT,
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,
		0.0f,
		16,
		D3D12_COMPARISON_FUNC_LESS_EQUAL,
		D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK);
#pragma endregion

	CD3DX12_DESCRIPTOR_RANGE texTable;
	texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	CD3DX12_ROOT_PARAMETER slotRootParam[4];
	slotRootParam[0].InitAsShaderResourceView(0, 1);
	slotRootParam[1].InitAsConstantBufferView(0);
	slotRootParam[2].InitAsConstantBufferView(1);
	slotRootParam[3].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);

	CD3DX12_ROOT_SIGNATURE_DESC rootDesc;
	rootDesc.Init(4, slotRootParam, _countof(staticSamplers),
		staticSamplers, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> error = nullptr;

	HRESULT hr = D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), error.GetAddressOf());

	if (error != nullptr)
	{
		::OutputDebugStringA((char*)error->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(m_D3dDevice->CreateRootSignature(0, serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(), IID_PPV_ARGS(m_RootSignature.GetAddressOf())));
}

void GraphicDX12::BuildShadersAndInputLayout()
{
	m_Shaders["baseVS"] = CompileShader(L"../Common/MainShaders/BaseShader.hlsl", nullptr, "VS", "vs_5_1");
	m_Shaders["basePS"] = CompileShader(L"../Common/MainShaders/BaseShader.hlsl", nullptr, "PS", "ps_5_1");

	m_NTVertexInputLayout =
	{
		{ "POSITION" ,0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

void GraphicDX12::BuildGeometry()
{
#pragma region Vertex setting for test

	vector<XMFLOAT3> offsetVertex;
	vector<Vertex> vertexData;
	vector<UINT> indexData;
	vector<UINT> vertexOffsets = { 0 };
	UINT numIndexs = 0;

	for (auto& it : m_XfileObject->m_GlobalMeshes)
	{
		UINT currIndexOffset = vertexOffsets.back();
		vertexOffsets.push_back(vertexOffsets.back() + it->m_Positions.size());

	/*	offsetVertex.clear();
		offsetVertex.resize(it->m_Positions.size());
		memset(&offsetVertex[0], 0, offsetVertex.size() * sizeof(XMFLOAT3));
		vector<float> weights(it->m_Positions.size());

		for (auto& it2 : it->m_Bones)
		{
			for (auto& it3 : it2.m_Weights)
			{
				weights[it3.m_Vertex] += it3.m_Weight;
				XMVECTOR pos =XMLoadFloat3(&it->m_Positions[it3.m_Vertex]);
				XMMATRIX mat = XMLoadFloat4x4(it2.m_OffsetMatrix);

				pos = (XMVector3TransformCoord(pos, mat)*it3.m_Weight) + XMLoadFloat3(&offsetVertex[it3.m_Vertex]);

				XMStoreFloat3(&offsetVertex[it3.m_Vertex], pos);
			}
		}*/

		for (int i = 0; i < it->m_Positions.size(); i++)
		{
			vertexData.push_back(Vertex(it->m_Positions[i], XMFLOAT3(0, 0, 0), XMFLOAT2(0, 0)));
		}

		for (auto& it2 : it->m_PosFaces)
		{
			for (auto& it3 : it2.m_Indices)
			{
				indexData.push_back(currIndexOffset + it3);

				if (indexData.back() >= vertexData.size())
				{
					assert(false);
				}
			}
		}
	}

#pragma endregion

	m_VertexBufferSize = static_cast<UINT>(vertexData.size()) * sizeof(Vertex);
	m_IndexBufferSize = static_cast<UINT>(indexData.size()) * sizeof(UINT);

	D3D12_HEAP_PROPERTIES uploadBufferPro = {};
	uploadBufferPro.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC bufferDesc = {};
	bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bufferDesc.DepthOrArraySize = 1;
	bufferDesc.Width = m_VertexBufferSize;
	bufferDesc.Height = 1;
	bufferDesc.MipLevels = 1;
	bufferDesc.SampleDesc.Count = 1;
	bufferDesc.SampleDesc.Quality = 0;
	bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	D3D12_HEAP_PROPERTIES vertexBufferPro = {};
	vertexBufferPro.Type = D3D12_HEAP_TYPE_DEFAULT;

	TIF_AND_SETNAME(m_D3dDevice->CreateCommittedResource(&uploadBufferPro, D3D12_HEAP_FLAG_NONE,
		&bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(m_VertexUploadBuffer.GetAddressOf())), m_VertexUploadBuffer);

	TIF_AND_SETNAME(m_D3dDevice->CreateCommittedResource(&vertexBufferPro, D3D12_HEAP_FLAG_NONE,
		&bufferDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
		IID_PPV_ARGS(m_VertexBuffer.GetAddressOf())), m_VertexBuffer);

	bufferDesc.Width = m_IndexBufferSize;

	TIF_AND_SETNAME(m_D3dDevice->CreateCommittedResource(&uploadBufferPro, D3D12_HEAP_FLAG_NONE,
		&bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(m_IndexUploadBuffer.GetAddressOf())), m_IndexUploadBuffer);

	TIF_AND_SETNAME(m_D3dDevice->CreateCommittedResource(&vertexBufferPro, D3D12_HEAP_FLAG_NONE,
		&bufferDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
		IID_PPV_ARGS(m_IndexBuffer.GetAddressOf())), m_IndexBuffer);

	void* dataTemp = nullptr;
	void* indexTemp = nullptr;
	ThrowIfFailed(m_VertexUploadBuffer->Map(0, nullptr, &dataTemp));
	ThrowIfFailed(m_IndexUploadBuffer->Map(0, nullptr, &indexTemp));

	::memcpy(dataTemp, vertexData.data(), m_VertexBufferSize);
	::memcpy(indexTemp, indexData.data(), m_IndexBufferSize);
	
	m_VertexUploadBuffer->Unmap(0, nullptr);
	m_IndexUploadBuffer->Unmap(0, nullptr);

	m_CommandList->CopyBufferRegion(m_VertexBuffer.Get(), 0,
		m_VertexUploadBuffer.Get(), 0, m_VertexBufferSize);
	m_CommandList->CopyBufferRegion(m_IndexBuffer.Get(), 0,
		m_IndexUploadBuffer.Get(), 0, m_IndexBufferSize);

	m_CommandList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(m_VertexBuffer.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

	m_CommandList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(m_IndexBuffer.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_INDEX_BUFFER));
}

void GraphicDX12::BuildPSOs()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;
	D3D12_RENDER_TARGET_BLEND_DESC transparencyBlendDesc;
	transparencyBlendDesc.BlendEnable = true;
	transparencyBlendDesc.LogicOpEnable = false;
	transparencyBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	transparencyBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	transparencyBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	transparencyBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	transparencyBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	transparencyBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	transparencyBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
	transparencyBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	opaquePsoDesc.InputLayout = { m_NTVertexInputLayout.data(), (UINT)m_NTVertexInputLayout.size() };
	opaquePsoDesc.pRootSignature = m_RootSignature.Get();
	opaquePsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(m_Shaders["baseVS"]->GetBufferPointer()),
		m_Shaders["baseVS"]->GetBufferSize()
	};
	opaquePsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(m_Shaders["basePS"]->GetBufferPointer()),
		m_Shaders["basePS"]->GetBufferSize()
	};
	opaquePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	opaquePsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	opaquePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	opaquePsoDesc.BlendState.RenderTarget[0] = transparencyBlendDesc;
	opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	opaquePsoDesc.SampleMask = UINT_MAX;
	opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	opaquePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	opaquePsoDesc.NumRenderTargets = 1;
	opaquePsoDesc.RTVFormats[0] = m_BackBufferFormat;
	opaquePsoDesc.SampleDesc.Count = m_4xMsaaState ? 4 : 1;
	opaquePsoDesc.SampleDesc.Quality = m_4xMsaaState ? (m_4xmsaaQuality - 1) : 0;
	opaquePsoDesc.DSVFormat = m_DepthStencilFormat;
	ThrowIfFailed(m_D3dDevice->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&m_PSOs["base"])));
}

void GraphicDX12::BuildRenderItem()
{
}

void GraphicDX12::UpdateMainPassCB()
{
	XMMATRIX view = XMLoadFloat4x4(m_ViewMatrix);
	XMMATRIX proj = XMLoadFloat4x4(m_ProjectionMat);

	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

	XMStoreFloat4x4(m_MainPassCB.view, XMMatrixTranspose(view));
	XMStoreFloat4x4(m_MainPassCB.invView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(m_MainPassCB.proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(m_MainPassCB.invProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(m_MainPassCB.viewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(m_MainPassCB.invViewProj, XMMatrixTranspose(invViewProj));
	m_MainPassCB.renderTargetSize = XMFLOAT2((float)m_ClientWidth, (float)m_ClientHeight);
	m_MainPassCB.invRenderTargetSize = XMFLOAT2(1.0f / m_ClientWidth, 1.0f / m_ClientHeight);

	m_MainPassCB.ambientLight = { 0.25f, 0.25f, 0.35f, 1.0f };
	//m_MainPassCB.Lights[0].direction = { 0.57735f, -0.57735f, 0.57735f };
	//m_MainPassCB.Lights[0].strength = { 0.9f, 0.9f, 0.9f };
	//m_MainPassCB.Lights[1].direction = { -0.57735f, -0.57735f, 0.57735f };
	//m_MainPassCB.Lights[1].strength = { 0.5f, 0.5f, 0.5f };
	//m_MainPassCB.Lights[2].direction = { 0.0f, -0.707f, -0.707f };
	//m_MainPassCB.Lights[2].strength = { 0.2f, 0.2f, 0.2f };

	m_FrameResource->passCB->CopyData(0, m_MainPassCB);
}

void GraphicDX12::UpdateObjects()
{

}

ComPtr<ID3DBlob> GraphicDX12::CompileShader(
	const std::wstring& filename,
	const D3D_SHADER_MACRO* defines,
	const std::string& entrypoint,
	const std::string& target)
{
	UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	HRESULT hr = S_OK;

	ComPtr<ID3DBlob> byteCode = nullptr;
	ComPtr<ID3DBlob> errors;
	hr = D3DCompileFromFile(filename.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		entrypoint.c_str(), target.c_str(), compileFlags, 0, &byteCode, &errors);

	if (errors != nullptr)
		OutputDebugStringA((char*)errors->GetBufferPointer());

	ThrowIfFailed(hr);

	return byteCode;
}
