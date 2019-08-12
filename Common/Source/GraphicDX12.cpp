#include "GraphicDX12.h"
#include "cCamera.h"
#include "cTextureHeap.h"
#include <functional>

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

	//디버그 세팅. 비주얼 스튜디오 출력창에 디버그 정보를 띠워준다.
#if defined(DEBUG)||defined(_DEBUG)
	{
		ComPtr<ID3D12Debug> debugController;
		ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(debugController.GetAddressOf())));
		debugController->EnableDebugLayer();
	}
#endif
	
	HRESULT hr = S_OK;
	
	//스왑 체인을 만들기 위해선 DXGI Factory가 필요함.
	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(m_DxgiFactory.GetAddressOf())));

	//D12 Device 생성
	hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(m_D3dDevice.GetAddressOf()));

	//feature Level을 지원하지 못하는 하드웨어(ex 내놋북)면 소프트웨어기반(cpu)의 WarpAdapter를 만든다
	if (FAILED(hr))
	{
		ComPtr<IDXGIAdapter> pWarpAdapter;
		ThrowIfFailed(m_DxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(pWarpAdapter.GetAddressOf())));

		ThrowIfFailed(D3D12CreateDevice(pWarpAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(m_D3dDevice.GetAddressOf())));
	}

	//cpu와 gpu 작업큐 동기화를 위한 펜스 생성
	ThrowIfFailed(m_D3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(m_Fence.GetAddressOf())));

	//각 자원의 서술자 크기를 미리 저장해둠. 여기저기에서 매개변수로 많이 쓰임
	m_RTVDescriptorSize = m_D3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_DSVDescriptorSize = m_D3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	m_CBV_SRV_UAV_DescriptorSize = m_D3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//멀티 샘플링 여부 체크
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;

	msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msQualityLevels.Format = m_BackBufferFormat;
	msQualityLevels.SampleCount = 4;
	msQualityLevels.NumQualityLevels = 0;

	ThrowIfFailed(m_D3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&msQualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS)));

	m_4xmsaaQuality = msQualityLevels.NumQualityLevels;
	assert(m_4xmsaaQuality > 0 && "Unexpected MSAA quality level.");

	// GPU에 실행 명령을 내리기 위한 오브젝트 생성( commandList, queue 등 )
	CreateCommandObject();

	// dx12에선 스왑체인을 직접 만들어야함
	CreateSwapChain();

	// 스왑체인에 쓰일 RenderTarget 서술자, DepthStencil 서술자가 담길 힙을 생성.
	CreateRtvAndDsvDescriptorHeaps();

	// 랜더타겟이 될 리소스를 현재 클라이언트 사이즈에 맞게 생성하고 스왑체인에 묶어준다
	OnResize();

#pragma region RenderObjectsBuild

	ThrowIfFailed(m_CommandList->Reset(m_DirectCmdListAlloc.Get(), nullptr));

	BuildTextures();
	BuildRootSignature();			// ****************** 제일 중요한거. 이부분 무조건 이해하고 넘어가야함
	BuildShadersAndInputLayout();	// 다렉9 Vertex fvf 생각하면 쉽게 이해할 수 있음.
	BuildPSOs();					// 렌더링 파이프 라인에 대한 설계도라고 생각하면 됨.
	BuildGeometry();				//
	BuildMaterials();				//
	BuildRenderItem();				//
	BuildFrameResources();			// FrameResource를 통해서 메모리의 데이터들(ViewMatrix같은)을 GPU에 업로드한다.
	
	ThrowIfFailed(m_CommandList->Close());
	ID3D12CommandList* cmdsLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	FlushCommandQueue();
#pragma endregion 

	return true;
}

void GraphicDX12::CreateCommandObject()
{
	/*	1. 커맨드 리스트를 Allocator, Pipeline State Obejct와 연결
		2. 커맨드 리스트에 명령들을 저장
		3. 커맨드 큐에 커맨드 리스트의 배열을 Excute
		4. 그래픽 디바이스에서 큐에서 하나씩 빼서 실행 */

		//실제로 디바이스에서 실행을 하는 명령들이 저장된 큐를 생성한다.
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

	// 스왑체인의 속성값 들을 지정한 뒤 DXGIFactory로 생성.
	// DX12에선 CommandQueue를 매개변수로 넣어주어야 함.
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
	// 랜더 타겟 서술자가 들어갈 힙 생성.
	// 스왑체인의 숫자에 맞는 크기로 생성한다 (현재 크기= SwapChainBufferCount(2))
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	rtvHeapDesc.NumDescriptors = SwapChainBufferCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

	// 깊이,스탠실 서술자용 힙 생성.
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
	// Command Queue를 비워준다
	FlushCommandQueue();

	// CommandList 초기화
	ThrowIfFailed(m_CommandList->Reset(m_DirectCmdListAlloc.Get(), nullptr));

	// 스왑체인에 사용될 리소스들 전부 릴리즈
	for (int i = 0; i < SwapChainBufferCount; i++)
	{
		m_SwapChainBuffer[i] = nullptr;
	}

	m_DepthStencilBuffer = nullptr;

	// 스왑체인 오브젝트의 크기 변경
	ThrowIfFailed(m_SwapChain->ResizeBuffers(SwapChainBufferCount,
		m_ClientWidth, m_ClientHeight, m_BackBufferFormat, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));
	m_CurrBackBuffer = 0;

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RTVHeap->GetCPUDescriptorHandleForHeapStart());

	for (int i = 0; i < SwapChainBufferCount; i++)
	{
		// 스왑체인 오브젝트로 부터 리소스를 받아온다
		ThrowIfFailed(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(m_SwapChainBuffer[i].GetAddressOf())));

		// 새로 받아온 리소스에 대한 서술자를 생성 및, rtvHandle(힙의 포인터) 위치에 담아준다
		m_D3dDevice->CreateRenderTargetView(m_SwapChainBuffer[i].Get(), nullptr, rtvHandle);

		//rtvHandle(힙의 포인터) 위치를 변경한다
		rtvHandle.Offset(1, m_RTVDescriptorSize);
	}

	// 깊이, 스탠실 버퍼를 생성한다.

	D3D12_RESOURCE_DESC dsvDesc;
	dsvDesc.Alignment = 0; //조정
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

	//생성된 깊이, 스탠실 버퍼의 서술자를 생성 및 서술자힙에 담아준다.
	m_D3dDevice->CreateDepthStencilView(m_DepthStencilBuffer.Get(), &dsvViewDesc,
		m_DSVHeap->GetCPUDescriptorHandleForHeapStart());

	//생성된 깊이, 스탠실버퍼의 상태를 데이터 기록 가능한 상태로 변경한다.
	m_CommandList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(m_DepthStencilBuffer.Get(),
			D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE));

	// CommandList에 모든 명령들을 기록했다면 Close()
	ThrowIfFailed(m_CommandList->Close());

	// CommandQueue에 CommandList의 배열의 포인터를 넘긴다(실행된다)
	ID3D12CommandList * cmdLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

	// 모든 실행이 될때까지 기다린다.
	FlushCommandQueue();

	m_ScreenViewport.TopLeftX = 0;
	m_ScreenViewport.TopLeftY = 0;
	m_ScreenViewport.Width = static_cast<float>(m_ClientWidth);
	m_ScreenViewport.Height = static_cast<float>(m_ClientHeight);
	m_ScreenViewport.MinDepth = 0.0f;
	m_ScreenViewport.MaxDepth = 1.0f;

	m_ScissorRect = { 0, 0, m_ClientWidth, m_ClientHeight };

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * XM_PI, (float)m_ClientWidth/m_ClientHeight, 1.0f, 1000.0f);
	XMStoreFloat4x4(m_ProjectionMat, P);
}

std::shared_ptr<IComponent> GraphicDX12::CreateComponent(PxTransform& trans)
{

	return std::shared_ptr<IComponent>();
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

	m_CommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	m_CommandList->SetGraphicsRootSignature(m_RootSignature.Get());

	auto matBuffer = m_FrameResource->materialBuffer->Resource();
	m_CommandList->SetGraphicsRootShaderResourceView(0, matBuffer->GetGPUVirtualAddress());

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
}

void GraphicDX12::BuildTextures()
{
	m_TextureHeap = make_unique<cTextureHeap>(m_D3dDevice.Get(), 1);

	m_TextureHeap->Begin(m_D3dDevice.Get());

	m_TextureHeap->AddTexture(m_D3dDevice.Get(),
		m_CommandQueue.Get(), "testTexture", L"./../Common/TextureData/ui.png");

	m_TextureHeap->End(m_CommandQueue.Get(), bind(&GraphicDX12::FlushCommandQueue, this));
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
	CD3DX12_DESCRIPTOR_RANGE texTable[2];
	texTable[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	texTable[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

	CD3DX12_ROOT_PARAMETER slotRootParam[4];
	slotRootParam[0].InitAsShaderResourceView(0, 1);
	slotRootParam[1].InitAsConstantBufferView(0);
	slotRootParam[2].InitAsConstantBufferView(1);
	slotRootParam[3].InitAsDescriptorTable(2, texTable, D3D12_SHADER_VISIBILITY_PIXEL);
	
	CD3DX12_ROOT_SIGNATURE_DESC rootDesc;
	rootDesc.Init(4, slotRootParam, 0,
		nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

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
	//opaquePsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
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
	m_MainPassCB.Lights[0].direction = { 0.57735f, -0.57735f, 0.57735f };
	m_MainPassCB.Lights[0].strength = { 0.9f, 0.9f, 0.9f };
	m_MainPassCB.Lights[1].direction = { -0.57735f, -0.57735f, 0.57735f };
	m_MainPassCB.Lights[1].strength = { 0.5f, 0.5f, 0.5f };
	m_MainPassCB.Lights[2].direction = { 0.0f, -0.707f, -0.707f };
	m_MainPassCB.Lights[2].strength = { 0.2f, 0.2f, 0.2f };
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
