#include "GraphicDX12.h"
#include "cCamera.h"
#include "DX12/cTextureBuffer.h"
#include "GraphicDO.h"
#include "BaseClass.h"

using namespace DirectX;
using namespace std;

GraphicDX12::GraphicDX12()
{
}

GraphicDX12::~GraphicDX12()
{
}

bool GraphicDX12::Init(HWND hWnd, UINT windowWidth, UINT windowHeight)
{
	m_MainWndHandle = hWnd;
	m_ClientWidth = windowWidth;
	m_ClientHeight = windowHeight;
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

	// GPU에 실행 명령을 내리기 위한 오브젝트 생성( commandList, queue 등 )
	CreateCommandObject();

	// dx12에선 스왑체인을 직접 만들어야함
	CreateSwapChain();

	// 스왑체인에 쓰일 RenderTarget 서술자, DepthStencil 서술자가 담길 힙을 생성.
	CreateRtvAndDsvDescriptorHeaps();

	// 랜더타겟이 될 리소스를 현재 클라이언트 사이즈에 맞게 생성하고 스왑체인에 묶어준다
	OnResize();

	m_PSOCon = make_unique<PSOController>(m_D3dDevice.Get());

	ThrowIfFailed(m_DirectCmdListAlloc->Reset());

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
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
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

	FlushCommandQueue();

	ThrowIfFailed(m_CommandList->Reset(m_DirectCmdListAlloc.Get(), nullptr));

	for (int i = 0; i < SwapChainBufferCount; i++)
	{
		m_SwapChainBuffer[i] = nullptr;
	}

	m_DepthStencilBuffer = nullptr;

	ThrowIfFailed(m_SwapChain->ResizeBuffers(SwapChainBufferCount,
		m_ClientWidth, m_ClientHeight, m_BackBufferFormat, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));
	m_CurrBackBuffer = 0;

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RTVHeap->GetCPUDescriptorHandleForHeapStart());

	for (int i = 0; i < SwapChainBufferCount; i++)
	{
		ThrowIfFailed(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(m_SwapChainBuffer[i].GetAddressOf())));

		m_D3dDevice->CreateRenderTargetView(m_SwapChainBuffer[i].Get(), nullptr, rtvHandle);

		rtvHandle.Offset(1, m_RTVDescriptorSize);
	}

	D3D12_RESOURCE_DESC dsvDesc;
	dsvDesc.Alignment = 0;
	dsvDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	dsvDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	dsvDesc.DepthOrArraySize = 1;
	dsvDesc.MipLevels = 1;
	dsvDesc.Width = m_ClientWidth;
	dsvDesc.Height = m_ClientHeight;
	dsvDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	dsvDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	dsvDesc.SampleDesc.Count = 1;
	dsvDesc.SampleDesc.Quality = 0;

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

	m_D3dDevice->CreateDepthStencilView(m_DepthStencilBuffer.Get(), &dsvViewDesc,
		m_DSVHeap->GetCPUDescriptorHandleForHeapStart());

	m_CommandList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(m_DepthStencilBuffer.Get(),
			D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE));

	ThrowIfFailed(m_CommandList->Close());

	ID3D12CommandList* cmdLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

	FlushCommandQueue();

	m_ScreenViewport.TopLeftX = 0;
	m_ScreenViewport.TopLeftY = 0;
	m_ScreenViewport.Width = static_cast<float>(m_ClientWidth);
	m_ScreenViewport.Height = static_cast<float>(m_ClientHeight);
	m_ScreenViewport.MinDepth = 0.0f;
	m_ScreenViewport.MaxDepth = 1.0f;

	m_ScissorRect = { 0, 0, m_ClientWidth, m_ClientHeight };

	if (m_FontManager)
	{
		m_FontManager->Resize(m_ClientWidth, m_ClientHeight);
	}

	XMMATRIX P = XMMatrixPerspectiveFovLH(XM_PIDIV4, (float)m_ClientWidth / m_ClientHeight, 1.0f, 1000.0f);
	XMMATRIX OrthoP = XMMatrixOrthographicOffCenterLH(m_ScissorRect.left, m_ScissorRect.right,
		m_ScissorRect.bottom, m_ScissorRect.top, D3D12_MIN_DEPTH, D3D12_MAX_DEPTH);
	XMStoreFloat4x4(m_ProjectionMat, P);
	XMStoreFloat4x4(m_OrthoProjectionMat, OrthoP);
}

void GraphicDX12::GetWorldRay(physx::PxVec3& origin, physx::PxVec3& ray) const
{
	origin = m_RayOrigin;
	ray = m_Ray;
}

void GraphicDX12::RegisterDeviceObject(CGHScene& scene, DeviceObject* gameObject)
{
	auto typeName = gameObject->GetTypeName();

	if (typeName == typeid(DORenderer).name())
	{
	}
	else if (typeName == typeid(DOFont).name())
	{

	}
	else if (typeName == typeid(DORenderMesh).name())
	{

	}
	else if (typeName == typeid(DOAnimator).name())
	{

	}
}

void GraphicDX12::UnRegisterDeviceObject(CGHScene& scene, DeviceObject* gameObject)
{

}

bool GraphicDX12::CreateMesh(const std::string& meshName, MeshObject& meshinfo,
	const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices)
{
	ComPtr<ID3D12CommandAllocator>		allocator;
	ComPtr<ID3D12GraphicsCommandList>	commandList;

	ThrowIfFailed(m_D3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(allocator.GetAddressOf())));
	ThrowIfFailed(m_D3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		allocator.Get(), nullptr, IID_PPV_ARGS(commandList.GetAddressOf())));

	if (m_Meshs.find(meshName) != m_Meshs.end())
	{
		return false;
	}

	UINT baseVertexLocation = 0;
	UINT numVertices = 0;
	UINT baseIndexLocation = 0;
	UINT numIndices = 0;

	ComPtr<ID3D12Resource> prevVertexBuffer = nullptr;
	ComPtr<ID3D12Resource> prevIndexBuffer = nullptr;

	switch (meshinfo.type)
	{
	case CGH::MESH_NORMAL:
	{
		baseVertexLocation = m_VertexBuffer->GetNumDatas();
		baseIndexLocation = m_IndexBuffer->GetNumDatas();
		numVertices = vertices.size();
		numIndices = indices.size();

		prevVertexBuffer = m_VertexBuffer->AddData(m_D3dDevice.Get(), commandList.Get(),
			numVertices, vertices.data());
		prevIndexBuffer = m_IndexBuffer->AddData(m_D3dDevice.Get(), commandList.Get(),
			numIndices, indices.data());
	}
	break;
	case CGH::MESH_SKINED:
	case CGH::MESH_NONE:
		return false;
		break;
	default:
		break;
	}

	commandList->Close();

	ID3D12CommandList* cmdLists[] = { commandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

	for (auto& it : meshinfo.subs)
	{
		it.second.vertexOffset += baseVertexLocation;
		it.second.indexOffset += baseIndexLocation;
	}

	m_Meshs.insert({ meshName, meshinfo });

	FlushCommandQueue();
	ThrowIfFailed(allocator->Reset());

	m_VertexBuffer->ClearUploadBuffer();
	m_IndexBuffer->ClearUploadBuffer();

	return true;
}

bool GraphicDX12::CreateMaterials(const std::vector<std::string>& materialNames, const std::vector<Material>& materials)
{
	ComPtr<ID3D12CommandAllocator>		allocator;
	ComPtr<ID3D12GraphicsCommandList>	commandList;

	ThrowIfFailed(m_D3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(allocator.GetAddressOf())));
	ThrowIfFailed(m_D3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		allocator.Get(), nullptr, IID_PPV_ARGS(commandList.GetAddressOf())));

	auto prevBuffer = m_Materials->IndexedAddData(m_D3dDevice.Get(), commandList.Get(), materials.size(), materials.data(), materialNames);

	commandList->Close();

	ID3D12CommandList* cmdLists[] = { commandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

	FlushCommandQueue();
	ThrowIfFailed(allocator->Reset());

	m_Materials->ClearUploadBuffer();

	return true;
}

bool GraphicDX12::EditMesh(const std::string& meshName, const std::vector<Vertex>& vertices)
{
	bool result = false;
	auto iter = m_Meshs.find(meshName);

	if (iter == m_Meshs.end())
	{
		return false;
	}

	ComPtr<ID3D12CommandAllocator>		allocator;
	ComPtr<ID3D12GraphicsCommandList>	commandList;

	ThrowIfFailed(m_D3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(allocator.GetAddressOf())));
	ThrowIfFailed(m_D3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		allocator.Get(), nullptr, IID_PPV_ARGS(commandList.GetAddressOf())));

	switch (iter->second.type)
	{
	case CGH::MESH_NORMAL:
	{
		result = m_VertexBuffer->EditDatas(m_D3dDevice.Get(), commandList.Get(),
			iter->second.GetStartVertexOffset(), vertices.size(), vertices.data());
	}
	break;
	case CGH::MESH_SKINED:
	case CGH::MESH_NONE:
		return false;
		break;
	default:
		break;
	}

	commandList->Close();

	ID3D12CommandList* cmdLists[] = { commandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

	FlushCommandQueue();
	ThrowIfFailed(allocator->Reset());

	m_VertexBuffer->ClearUploadBuffer();

	return result;
}

bool GraphicDX12::EditMaterial(const std::string& materialName, const Material& material)
{
	bool result = false;

	ComPtr<ID3D12CommandAllocator>		allocator;
	ComPtr<ID3D12GraphicsCommandList>	commandList;

	ThrowIfFailed(m_D3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(allocator.GetAddressOf())));
	ThrowIfFailed(m_D3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		allocator.Get(), nullptr, IID_PPV_ARGS(commandList.GetAddressOf())));

	Material temp = material;

	result = m_Materials->EditData(m_D3dDevice.Get(), commandList.Get(), materialName, &temp);

	commandList->Close();

	ID3D12CommandList* cmdLists[] = { commandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

	FlushCommandQueue();
	ThrowIfFailed(allocator->Reset());

	m_Materials->ClearUploadBuffer();

	return result;
}

bool GraphicDX12::CreateDynamicVIBuffer(unsigned int vertexNum, unsigned int indexNum, DynamicBufferInfo** out)
{
	DynamicBuffer* addedDyanamic = nullptr;

	if (*out)
	{
		for (auto& it : m_DynamicBuffers)
		{
			if (it->dynamicBufferInfo.get() == *out)
			{
				return false;
			}
		}
	}

	for (size_t i = 0; i < m_DynamicBuffers.size(); i++)
	{
		if (m_DynamicBuffers[i] == nullptr)
		{
			m_DynamicBuffers[i] = make_unique<DynamicBuffer>(m_D3dDevice.Get(), i, vertexNum, indexNum);

			addedDyanamic = m_DynamicBuffers[i].get();
			break;
		}
	}

	if (addedDyanamic == nullptr)
	{
		m_DynamicBuffers.emplace_back(make_unique<DynamicBuffer>(m_D3dDevice.Get(),
			m_DynamicBuffers.size(), vertexNum, indexNum));
		addedDyanamic = m_DynamicBuffers.back().get();
	}

	if (addedDyanamic)
	{
		*out = addedDyanamic->dynamicBufferInfo.get();

		return true;
	}
	else
	{
		return false;
	}
}

void GraphicDX12::EditDynamicVIBuffer(const DynamicBufferInfo* dvi, DYNAMIC_BUFFER_EDIT_MOD mode, const std::vector<float>& inputDatas)
{

}

void GraphicDX12::SaveAndMergeDynamicVIBufferToDefaultVertexBuffer(const std::string& meshName, const DynamicBufferInfo* dvi)
{

}

void GraphicDX12::ReleaseDynamicVIBuffer(const DynamicBufferInfo* dvi)
{
	if (dvi)
	{
		for (size_t i = 0; i < m_DynamicBuffers.size(); i++)
		{
			if (m_DynamicBuffers[i].get()->dynamicBufferInfo.get() == dvi)
			{
				m_DynamicBuffers[i] = nullptr;
				break;
			}
		}
	}
}

int GraphicDX12::GetTextureIndex(const std::string& textureName)
{
	return m_TextureBuffer->GetTextureIndex(textureName);
}

void GraphicDX12::LoadTextureFromFolder(const std::vector<std::wstring>& targetTextureFolders)
{
	vector<string> files;
	for (auto& it : targetTextureFolders)
	{
		SearchAllFileFromFolder(it, true, files);
	}

	unordered_map<string, wstring> texTurePaths;
	for (auto& it : files)
	{
		string extension;
		string fileName = GetFileNameFromPath(it, extension);
		wstring temp(it.begin(), it.end());

		if (CheckFileExtension(extension) == EXTENSIONTYPE::EXE_TEXTURE)
		{
			texTurePaths[fileName] = temp;
		}
	}

	const UINT numTexturePath = texTurePaths.size();
	m_TextureBuffer = make_unique<cTextureBuffer>(m_D3dDevice.Get(), numTexturePath);

	m_TextureBuffer->Begin(m_D3dDevice.Get());

	for (auto& it : texTurePaths)
	{
		m_TextureBuffer->AddTexture(m_D3dDevice.Get(),
			m_CommandQueue.Get(), it.second);
	}

	auto test1 = bind(&GraphicDX12::FlushCommandQueue, this);
	function<void()> test = test1;
	m_TextureBuffer->End(m_CommandQueue.Get(), bind(&GraphicDX12::FlushCommandQueue, this));
}

void GraphicDX12::LoadMeshAndMaterialFromFolder(const std::vector<std::wstring>& targetMeshFolders)
{
	ThrowIfFailed(m_CommandList->Reset(m_DirectCmdListAlloc.Get(), nullptr));

	vector<string> files;

	for (auto& it : targetMeshFolders)
	{
		SearchAllFileFromFolder(it, true, files);
	}

	XFileParser xfileP;
	vector<Vertex> combinedVertex;
	vector<unsigned int> combinedIndices;

	vector<SkinnedVertex> combinedSkinnedVertex;
	vector<unsigned int> combinedSkinnedIndices;

	vector<Material> matDatas;
	vector<string> matNames;

	vector<SkinnedVertex> skinnedVertices;
	vector<Vertex> vertices;
	vector<unsigned int> indices;

	//Instance for only XFile load
	vector<Ani::Subset> subsets;
	vector<Ani::AniMaterial> mats;
	Ani::SkinnedData skinnedData;

	for (auto& it : files)
	{
		MeshObject meshObject;
		string extension;
		string fileName = GetFileNameFromPath(it, extension);
		wstring temp(it.begin(), it.end());
		indices.clear();

		UINT baseVertexOffset = 0;
		UINT baseIndexOffset = 0;
		meshObject.primitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		skinnedVertices.clear();
		vertices.clear();

		if (extension == "X")
		{
			meshObject.type = CGH::MESH_SKINED;
			baseVertexOffset = static_cast<UINT>(combinedSkinnedVertex.size());
			baseIndexOffset = static_cast<UINT>(combinedSkinnedIndices.size());

			xfileP.LoadXfile(it, skinnedVertices, indices, subsets, mats, skinnedData);

			for (auto& it2 : subsets)
			{
				UINT faceCount = 0;

				for (auto& materalIndex : it2.materialIndexCount)
				{
					SubmeshData sub;
					sub.material = materalIndex.first;
					sub.indexOffset = it2.indexStart + baseIndexOffset + faceCount;
					sub.numIndex = materalIndex.second;
					sub.vertexOffset = baseVertexOffset;
					sub.numVertex = it2.vertexCount;

					faceCount += materalIndex.second;
					meshObject.subs.insert({ materalIndex.first ,sub });
				}
			}

			for (auto& it2 : mats)
			{
				Material material;
				for (auto& textureIter : it2.textures)
				{
					if (textureIter.isNormalMap)
					{
						meshObject.subs[it2.name].normalMap = textureIter.name;
					}
					else
					{
						meshObject.subs[it2.name].diffuseMap = textureIter.name;
					}
				}

				material.diffuseAlbedo = it2.diffuse;
				material.fresnel0 = it2.specular;
				material.roughness = it2.specularExponent;
				matDatas.push_back(material);
				matNames.push_back(it2.name);
			}

			if (!skinnedData.GetAnimationNum())
			{
				meshObject.type = CGH::MESH_NORMAL;
				//#TODO
			}

			m_SkinnedDatas.insert({ fileName, skinnedData });
			m_Meshs.insert({ fileName, meshObject });
		}

		if (meshObject.type == CGH::MESH_SKINED)
		{
			combinedSkinnedVertex.insert(combinedSkinnedVertex.end(), skinnedVertices.begin(), skinnedVertices.end());
			combinedSkinnedIndices.insert(combinedSkinnedIndices.end(), indices.begin(), indices.end());
		}
		else
		{
			combinedVertex.insert(combinedVertex.end(), vertices.begin(), vertices.end());
			combinedIndices.insert(combinedIndices.end(), indices.begin(), indices.end());
		}
	}


	m_VertexBuffer = make_unique<cDefaultBuffer<Vertex>>(m_D3dDevice.Get(),
		m_CommandList.Get(), combinedVertex, D3D12_RESOURCE_STATE_GENERIC_READ);

	m_IndexBuffer = make_unique<cDefaultBuffer<UINT>>(m_D3dDevice.Get(),
		m_CommandList.Get(), combinedIndices, D3D12_RESOURCE_STATE_GENERIC_READ);

	m_SkinnedVertexBuffer = make_unique<cDefaultBuffer<SkinnedVertex>>(m_D3dDevice.Get(),
		m_CommandList.Get(), combinedSkinnedVertex, D3D12_RESOURCE_STATE_GENERIC_READ);

	m_SkinnedIndexBuffer = make_unique<cDefaultBuffer<UINT>>(m_D3dDevice.Get(),
		m_CommandList.Get(), combinedSkinnedIndices, D3D12_RESOURCE_STATE_GENERIC_READ);


	m_Materials = make_unique<cIndexManagementBuffer<Material>>(m_D3dDevice.Get(),
		m_CommandList.Get(), matNames, matDatas);

	ThrowIfFailed(m_CommandList->Close());
	ID3D12CommandList* cmdsLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
	FlushCommandQueue();

	ThrowIfFailed(m_DirectCmdListAlloc->Reset());
	m_Materials->ClearUploadBuffer();
	m_VertexBuffer->ClearUploadBuffer();
	m_IndexBuffer->ClearUploadBuffer();
	m_SkinnedVertexBuffer->ClearUploadBuffer();
	m_SkinnedIndexBuffer->ClearUploadBuffer();
}

void GraphicDX12::LoadFontFromFolder(const std::vector<std::wstring>& targetFontFolders)
{
	vector<wstring> files;
	for (auto& it : targetFontFolders)
	{
		SearchAllFileFromFolder(it, true, files);
	}

	m_FontManager = make_unique<DX12FontManager>();
	m_FontManager->Init(m_D3dDevice.Get(), m_CommandQueue.Get(),
		files, m_BackBufferFormat, m_DepthStencilFormat);

	m_FontManager->Resize(m_ClientWidth, m_ClientHeight);
}

void GraphicDX12::LoadAniTreeFromFolder(const std::wstring& targetFolder)
{
	vector<wstring> files;
	SearchAllFileFromFolder(targetFolder, true, files);

	for (auto& it : files)
	{
		std::wstring extension;
		std::wstring wfileName = GetFileNameFromPath(it, extension);
		std::string fileName(wfileName.begin(), wfileName.end());

		if (extension == L"anitree")
		{
			if (m_AniTreeDatas.find(fileName) == m_AniTreeDatas.end())
			{
				m_AniTreeDatas[fileName] = make_unique<AniTree::AnimationTree>();
				m_AniTreeDatas[fileName]->LoadTree(it);
			}
			else
			{
				assert(false);
			}
		}
	}
}

void GraphicDX12::ReadyWorksEnd()
{
	ThrowIfFailed(m_CommandList->Reset(m_DirectCmdListAlloc.Get(), nullptr));

	BuildFrameResources();
	BuildRootSignature();
	BuildShadersAndInputLayout();
	BuildDepthStencilAndBlendsAndRasterizer();
	BuildUploadBuffers();

	ThrowIfFailed(m_CommandList->Close());
	ID3D12CommandList* cmdsLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
	FlushCommandQueue();
	ThrowIfFailed(m_DirectCmdListAlloc->Reset());
}

void GraphicDX12::FlushCommandQueue()
{
	m_CurrentFence++;

	ThrowIfFailed(m_CommandQueue->Signal(m_Fence.Get(), m_CurrentFence));

	if (m_Fence->GetCompletedValue() < m_CurrentFence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);

		ThrowIfFailed(m_Fence->SetEventOnCompletion(m_CurrentFence, eventHandle));

		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}

void GraphicDX12::Update(const CGHScene& scene)
{
	if (m_CurrCamera)
	{
		m_ViewMatrix = *m_CurrCamera->GetViewMatrix();
	}

	UpdateMainPassCB();
	UpdateObjectCB();
	UpdateAniBoneBuffer();
}

void GraphicDX12::Draw()
{
	auto cmdListAlloc = m_FrameResource->cmdListAlloc.Get();

	ThrowIfFailed(cmdListAlloc->Reset());

	ThrowIfFailed(m_CommandList->Reset(cmdListAlloc, nullptr));

	m_CommandList->RSSetViewports(1, &m_ScreenViewport);
	m_CommandList->RSSetScissorRects(1, &m_ScissorRect);

	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	m_CommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	m_CommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::Gray, 0, nullptr);
	m_CommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	DrawObject(DX12_RENDER_TYPE_NORMAL_MESH);
	DrawObject(DX12_RENDER_TYPE_DYNAMIC_MESH);
	DrawObject(DX12_RENDER_TYPE_SKINNED_MESH);
	DrawObject(DX12_RENDER_TYPE_POINT);
	DrawObject(DX12_RENDER_TYPE_UI);

	m_FontManager->RenderCommandWrite(m_CommandList.Get(), m_ReservedFonts);

	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
	ThrowIfFailed(m_CommandList->Close());
	ID3D12CommandList* cmdsLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
	m_FontManager->Commit(m_CommandQueue.Get());

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
	m_FrameResource = make_unique<FrameResource>(m_D3dDevice.Get(), 1, 100, BONEMAXMATRIX * 100);
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
	texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, m_TextureBuffer->GetTexturesNum(), 0);

	CD3DX12_ROOT_PARAMETER baseRootParam[T1_ROOT_COUNT];
	baseRootParam[T1_MATERIAL_SRV].InitAsShaderResourceView(0, 1);
	baseRootParam[T1_PASS_CB].InitAsConstantBufferView(0);
	baseRootParam[T1_OBJECT_CB].InitAsConstantBufferView(1);
	baseRootParam[T1_ANIBONE_CB].InitAsConstantBufferView(2);
	baseRootParam[T1_TEXTURE_TABLE].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);

	CD3DX12_ROOT_SIGNATURE_DESC rootDesc;
	rootDesc.Init(T1_ROOT_COUNT, baseRootParam, _countof(staticSamplers),
		staticSamplers, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	///////////////////////////////////////////////////////////////////////////
	CD3DX12_ROOT_PARAMETER pointRenderRootParam[P1_ROOT_COUNT];
	pointRenderRootParam[P1_OBJECT_SRV].InitAsShaderResourceView(0, 1);
	pointRenderRootParam[P1_PASS_CB].InitAsConstantBufferView(0);
	pointRenderRootParam[P1_TEXTURE_TABLE].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);

	CD3DX12_ROOT_SIGNATURE_DESC pointRenderrootDesc;
	pointRenderrootDesc.Init(P1_ROOT_COUNT, pointRenderRootParam, _countof(staticSamplers),
		staticSamplers, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	///////////////////////////////////////////////////////////////////////////

	CD3DX12_ROOT_PARAMETER uiRenderRootparam[UI_ROOT_COUNT];
	uiRenderRootparam[UI_PASS_CB].InitAsConstantBufferView(0);
	uiRenderRootparam[UI_UIPASS_CB].InitAsConstantBufferView(1);
	uiRenderRootparam[UI_TEXTURE_TABLE].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);

	CD3DX12_ROOT_SIGNATURE_DESC uiRenderRootDesc;
	uiRenderRootDesc.Init(UI_ROOT_COUNT, uiRenderRootparam, _countof(staticSamplers),
		staticSamplers, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	m_PSOCon->AddRootSignature("T1", rootDesc);
	m_PSOCon->AddRootSignature("P1", pointRenderrootDesc);
	m_PSOCon->AddRootSignature("UI", uiRenderRootDesc);
}

void GraphicDX12::BuildShadersAndInputLayout()
{
	string textureNum = to_string(m_TextureBuffer->GetTexturesNum());
	string boneMaxMatrixNum = to_string(BONEMAXMATRIX);
	D3D_SHADER_MACRO macros[] = {
		"MAXTEXTURE", textureNum.c_str(),
		"BONEMAXMATRIX", boneMaxMatrixNum.c_str(),
		"SKINNED_VERTEX_SAHDER",NULL,
		NULL, NULL };

	m_PSOCon->AddShader(to_string(DX12_RENDER_TYPE_SKINNED_MESH), DX12_SHADER_VERTEX, L"../Common/MainShaders/BaseShader.hlsl", macros, "VS", "vs_5_1");
	m_PSOCon->AddShader(to_string(DX12_RENDER_TYPE_SKINNED_MESH), DX12_SHADER_PIXEL, L"../Common/MainShaders/BaseShader.hlsl", macros, "PS", "ps_5_1");

	macros[2] = { NULL, NULL };
	m_PSOCon->AddShader(to_string(DX12_RENDER_TYPE_NORMAL_MESH), DX12_SHADER_VERTEX, L"../Common/MainShaders/BaseShader.hlsl", macros, "VS", "vs_5_1");
	m_PSOCon->AddShader(to_string(DX12_RENDER_TYPE_NORMAL_MESH), DX12_SHADER_PIXEL, L"../Common/MainShaders/BaseShader.hlsl", macros, "PS", "ps_5_1");

	m_PSOCon->AddShader(to_string(DX12_RENDER_TYPE_POINT), DX12_SHADER_VERTEX, L"../Common/MainShaders/pointShader.hlsl", macros, "VS", "vs_5_1");
	m_PSOCon->AddShader(to_string(DX12_RENDER_TYPE_POINT), DX12_SHADER_PIXEL, L"../Common/MainShaders/pointShader.hlsl", macros, "PS", "ps_5_1");
	m_PSOCon->AddShader(to_string(DX12_RENDER_TYPE_POINT), DX12_SHADER_GEOMETRY, L"../Common/MainShaders/pointShader.hlsl", macros, "GS", "gs_5_1");

	m_PSOCon->AddShader(to_string(DX12_RENDER_TYPE_UI), DX12_SHADER_VERTEX, L"../Common/MainShaders/UIShader.hlsl", macros, "VS", "vs_5_1");
	m_PSOCon->AddShader(to_string(DX12_RENDER_TYPE_UI), DX12_SHADER_PIXEL, L"../Common/MainShaders/UIShader.hlsl", macros, "PS", "ps_5_1");
	m_PSOCon->AddShader(to_string(DX12_RENDER_TYPE_UI), DX12_SHADER_GEOMETRY, L"../Common/MainShaders/UIShader.hlsl", macros, "GS", "gs_5_1");

	m_PSOCon->AddInputLayout(to_string(DX12_RENDER_TYPE_SKINNED_MESH),
		{
			{ "POSITION" ,0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "WEIGHTS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "BONEINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT, 0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		});

	m_PSOCon->AddInputLayout(to_string(DX12_RENDER_TYPE_NORMAL_MESH),
		{
			{ "POSITION" ,0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		});

	m_PSOCon->AddInputLayout(to_string(DX12_RENDER_TYPE_POINT),
		{
			{ "MESHTYPE" ,0,DXGI_FORMAT_R32_UINT,0,0,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
			{ "CBINDEX", 0, DXGI_FORMAT_R32_UINT, 0, 4, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "MESHSIZE", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 8, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "MESHCOLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		});

	m_PSOCon->AddInputLayout(to_string(DX12_RENDER_TYPE_UI),
		{
			{ "UICOLOR" ,0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,0,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
			{ "UIPOS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "UISIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "UITYPE", 0, DXGI_FORMAT_R32_SINT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "UITEXTURE", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 40, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		});
}

void GraphicDX12::BuildDepthStencilAndBlendsAndRasterizer()
{
	CD3DX12_BLEND_DESC baseBlendDesc(D3D12_DEFAULT);
	D3D12_RENDER_TARGET_BLEND_DESC baseBlendTargetDesc;
	baseBlendTargetDesc.BlendEnable = true;
	baseBlendTargetDesc.LogicOpEnable = false;
	baseBlendTargetDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	baseBlendTargetDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	baseBlendTargetDesc.BlendOp = D3D12_BLEND_OP_ADD;
	baseBlendTargetDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	baseBlendTargetDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	baseBlendTargetDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	baseBlendTargetDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
	baseBlendTargetDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	
	baseBlendDesc.RenderTarget[0] = baseBlendTargetDesc;
	baseBlendDesc.AlphaToCoverageEnable = true;

	m_PSOCon->AddBlend("0", baseBlendDesc);

	//////////////////////////////////////////////////////////////////////////
	CD3DX12_RASTERIZER_DESC baseRasterizer(D3D12_DEFAULT);
	baseRasterizer.CullMode = D3D12_CULL_MODE_BACK;

	m_PSOCon->AddRasterizer("0", baseRasterizer);

	////////////////////////////////////////////////////////////////////////////
	CD3DX12_DEPTH_STENCIL_DESC baseDepthStencil(D3D12_DEFAULT);
	m_PSOCon->AddDepthStencil("0", baseDepthStencil);
}

void GraphicDX12::BuildUploadBuffers()
{
	m_Box_Plane_Vertices = make_unique<UploadBuffer<B_P_Vertex>>(m_D3dDevice.Get(), 100, false);
	m_UIInfomation = make_unique<UploadBuffer<UIInfomation>>(m_D3dDevice.Get(), 150, false);
}

void GraphicDX12::UpdateMainPassCB()
{
	physx::PxMat44 view = m_ViewMatrix;
	physx::PxMat44 proj = m_ProjectionMat;

	physx::PxMat44 viewProj;

	XMMATRIX xmView = XMLoadFloat4x4(view);
	XMMATRIX xmProj = XMLoadFloat4x4(proj);
	XMStoreFloat4x4(viewProj, XMMatrixMultiply(xmView, xmProj));

	XMVECTOR deter;
	physx::PxMat44 invView;
	XMStoreFloat4x4(invView, XMMatrixInverse(&deter, xmView));
	physx::PxMat44 invProj = proj.inverseRT();
	physx::PxMat44 invViewProj = viewProj.inverseRT();

	m_MainPassCB.view = view.getTranspose();
	m_MainPassCB.invView = invView.getTranspose();
	m_MainPassCB.proj = proj.getTranspose();
	m_MainPassCB.invProj = invProj.getTranspose();
	m_MainPassCB.viewProj = viewProj.getTranspose();
	m_MainPassCB.invViewProj = invViewProj.getTranspose();
	m_MainPassCB.orthoMatrix = m_OrthoProjectionMat.getTranspose();
	m_MainPassCB.renderTargetSize = physx::PxVec2((float)m_ClientWidth, (float)m_ClientHeight);
	m_MainPassCB.invRenderTargetSize = physx::PxVec2(1.0f / m_ClientWidth, 1.0f / m_ClientHeight);
	m_MainPassCB.samplerIndex = CGH::GO.graphic.samplerIndex;

	m_MainPassCB.ambientLight = { 0.25f, 0.25f, 0.35f, 1.0f };
	//m_MainPassCB.Lights[0].direction = { 0.57735f, -0.57735f, 0.57735f };
	//m_MainPassCB.Lights[0].strength = { 0.9f, 0.9f, 0.9f };
	//m_MainPassCB.Lights[1].direction = { -0.57735f, -0.57735f, 0.57735f };
	//m_MainPassCB.Lights[1].strength = { 0.5f, 0.5f, 0.5f };
	//m_MainPassCB.Lights[2].direction = { 0.0f, -0.707f, -0.707f };
	//m_MainPassCB.Lights[2].strength = { 0.2f, 0.2f, 0.2f };

	m_FrameResource->passCB->CopyData(0, m_MainPassCB);
	m_FrameResource->uiPassCB->CopyData(0, CGH::GO.ui);

	physx::PxVec3 rayOrigin(0, 0, 0);
	physx::PxVec3 ray(0, 0, 1);

	if (m_CurrCamera)
	{
		ray = m_CurrCamera->GetViewRay(m_ProjectionMat, m_ClientWidth, m_ClientHeight);
	}

	rayOrigin = invView.transform(rayOrigin);
	ray = invView.rotate(ray).getNormalized();

	m_RayOrigin = rayOrigin;
	m_Ray = ray;
}

void GraphicDX12::UpdateObjectCB()
{
	for (int i = 0; i < DX12_RENDER_TYPE_COUNT; i++)
	{
		m_RenderObjects[i].clear();
		m_RenderObjectsSubmesh[i].clear();
	}

	m_NumRenderPointObjects = 0;
	m_NumRenderUIs = 0;

	auto pointCB = m_FrameResource->pointCB.get();

	for (auto& it : m_ReservedRenderInfos)
	{
		switch (it.type)
		{
		case RENDER_MESH:
		{
			const MeshObject& mesh = m_Meshs.find(it.meshOrTextureName)->second;
			ObjectConstants object;
			object.world = it.world;
			object.scale = it.scale;
			object.aniBoneIndex = it.mesh.aniBoneIndex;
			object.primitive = mesh.primitiveType;

			if (mesh.type == CGH::MESH_NORMAL)
			{
				for (auto& it2 : mesh.subs)
				{
					object.diffuseMapIndex = m_TextureBuffer->GetTextureIndex(it2.second.diffuseMap);
					object.normalMapIndex = m_TextureBuffer->GetTextureIndex(it2.second.normalMap);
					object.materialIndex = m_Materials->GetIndex(it2.second.material);
					m_RenderObjects[DX12_RENDER_TYPE_NORMAL_MESH].push_back(object);
					m_RenderObjectsSubmesh[DX12_RENDER_TYPE_NORMAL_MESH].push_back(&it2.second);
				}
			}
			else if (mesh.type == CGH::MESH_SKINED)
			{
				for (auto& it2 : mesh.subs)
				{
					object.diffuseMapIndex = m_TextureBuffer->GetTextureIndex(it2.second.diffuseMap);
					object.normalMapIndex = m_TextureBuffer->GetTextureIndex(it2.second.normalMap);
					object.materialIndex = m_Materials->GetIndex(it2.second.material);
					m_RenderObjects[DX12_RENDER_TYPE_SKINNED_MESH].push_back(object);
					m_RenderObjectsSubmesh[DX12_RENDER_TYPE_SKINNED_MESH].push_back(&it2.second);
				}
			}
		}
		break;
		case RENDER_DYNAMIC:
		{
			ObjectConstants object;
			object.world = it.world;
			object.scale = it.scale;
			object.primitive = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			object.dynamicBufferIndex = atoi(it.meshOrTextureName.c_str());

			const MeshObject& mesh = m_DynamicBuffers[object.dynamicBufferIndex]->dynamicBufferInfo->meshObject;
			for (auto& it2 : mesh.subs)
			{
				object.diffuseMapIndex = m_TextureBuffer->GetTextureIndex(it2.second.diffuseMap);
				object.normalMapIndex = m_TextureBuffer->GetTextureIndex(it2.second.normalMap);
				object.materialIndex = m_Materials->GetIndex(it2.second.material);
				m_RenderObjects[DX12_RENDER_TYPE_DYNAMIC_MESH].push_back(object);
				m_RenderObjectsSubmesh[DX12_RENDER_TYPE_DYNAMIC_MESH].push_back(&it2.second);
			}
		}
		break;
		case RENDER_BOX:
		case RENDER_PLANE:
		{
			B_P_Vertex temp;
			OnlyTexObjectConstants OTObjectConstnat;

			temp.type = it.type;
			temp.cbIndex = m_NumRenderPointObjects;
			temp.color = it.point.color;
			temp.size = it.point.size;
			m_Box_Plane_Vertices->CopyData(m_NumRenderPointObjects, &temp);

			OTObjectConstnat.world = it.world;
			pointCB->CopyData(m_NumRenderPointObjects, &OTObjectConstnat);
			m_NumRenderPointObjects++;
		}
		break;
		case RENDER_TEX_PLANE:
		{
			B_P_Vertex temp;
			OnlyTexObjectConstants OTObjectConstnat;

			temp.type = it.type;
			temp.cbIndex = m_NumRenderPointObjects;
			temp.size = it.texPoint.size;

			OTObjectConstnat.world = it.world;

			if (it.meshOrTextureName.size())
			{
				OTObjectConstnat.textureIndex = m_TextureBuffer->GetTextureIndex(it.meshOrTextureName);
			}

			if (OTObjectConstnat.textureIndex == -1)
			{
				temp.color = it.point.color;
			}

			m_Box_Plane_Vertices->CopyData(m_NumRenderPointObjects, &temp);
			pointCB->CopyData(m_NumRenderPointObjects, &OTObjectConstnat);
			m_NumRenderPointObjects++;
		}
		break;
		case RENDER_UI:
		{
			UIInfomation temp;
			temp.uiType = it.uiInfo.uiType;
			temp.size =it.uiInfo.size;
			temp.pos = it.world.getPosition();

			if (it.meshOrTextureName.size())
			{
				temp.textureIndex = m_TextureBuffer->GetTextureIndex(it.meshOrTextureName);
			}
			else
			{
				temp.color = it.uiInfo.color;
			}

			m_UIInfomation->CopyData(m_NumRenderUIs, &temp);
			m_NumRenderUIs++;
		}
		break;
		default:
			assert(false);
			break;
		}
	}

	for (size_t i = 0; i < DX12_RENDER_TYPE_POINT; i++)
	{
		auto currMeshOBCB = m_FrameResource->meshObjectCB[i].get();
		auto& currRenderObjects = m_RenderObjects[i];
		const size_t numRenders = currRenderObjects.size();

		for (size_t j = 0; j < numRenders; j++)
		{
			currMeshOBCB->CopyData(j, currRenderObjects[j]);
		}
	}
}

void GraphicDX12::UpdateAniBoneBuffer()
{
	auto aniBoneBuffer = m_FrameResource->aniBoneMatBuffer.get();

	UINT index = 0;
	for (auto& it : m_ReservedAniBones)
	{
		aniBoneBuffer->CopyData(index++, it);
	}
}

void GraphicDX12::DrawObject(DX12_RENDER_TYPE type)
{
	switch (type)
	{
	case GraphicDX12::DX12_RENDER_TYPE_NORMAL_MESH:
		DrawNormalMesh();
		break;
	case GraphicDX12::DX12_RENDER_TYPE_SKINNED_MESH:
		DrawSkinnedMesh();
		break;
	case GraphicDX12::DX12_RENDER_TYPE_DYNAMIC_MESH:
		DrawDynamicMehs();
		break;
	case GraphicDX12::DX12_RENDER_TYPE_POINT:
		DrawPointObjects();
		break;
	case GraphicDX12::DX12_RENDER_TYPE_UI:
		DrawUIs();
		break;
	default:
		break;
	}
}

void GraphicDX12::DrawNormalMesh()
{
	m_PSOCon->SetPSOToCommnadList(m_CommandList.Get(),
		{ m_BackBufferFormat }, m_DepthStencilFormat, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
		to_string(DX12_RENDER_TYPE_NORMAL_MESH), "T1", "0", "0", "0",
		to_string(DX12_RENDER_TYPE_NORMAL_MESH), 
		to_string(DX12_RENDER_TYPE_NORMAL_MESH));

	ID3D12DescriptorHeap* descriptorHeaps[] = { m_TextureBuffer->GetHeap() };
	m_CommandList->SetDescriptorHeaps(1, descriptorHeaps);

	auto matBuffer = m_Materials->GetBufferResource();
	m_CommandList->SetGraphicsRootShaderResourceView(T1_MATERIAL_SRV, matBuffer->GetGPUVirtualAddress());
	m_CommandList->SetGraphicsRootConstantBufferView(T1_PASS_CB, m_FrameResource->passCB->Resource()->GetGPUVirtualAddress());
	m_CommandList->SetGraphicsRootDescriptorTable(T1_TEXTURE_TABLE, m_TextureBuffer->GetHeap()->GetGPUDescriptorHandleForHeapStart());

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
	D3D12_INDEX_BUFFER_VIEW indexBufferView = {};
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;

	auto ObjectCBVritualAD = m_FrameResource->meshObjectCB[DX12_RENDER_TYPE_NORMAL_MESH]->Resource()->GetGPUVirtualAddress();
	const UINT ObjectStrideSize = m_FrameResource->meshObjectCB[DX12_RENDER_TYPE_NORMAL_MESH]->GetElementByteSize();

	vertexBufferView.BufferLocation = m_VertexBuffer->GetBufferResource()->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = m_VertexBuffer->GetBufferSize();
	vertexBufferView.StrideInBytes = sizeof(Vertex);

	indexBufferView.BufferLocation = m_IndexBuffer->GetBufferResource()->GetGPUVirtualAddress();
	indexBufferView.SizeInBytes = m_IndexBuffer->GetBufferSize();

	m_CommandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	m_CommandList->IASetIndexBuffer(&indexBufferView);
	m_CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	auto& currRenderObjects = m_RenderObjects[DX12_RENDER_TYPE_NORMAL_MESH];
	auto& currRenderObjectSubmesh = m_RenderObjectsSubmesh[DX12_RENDER_TYPE_NORMAL_MESH];

	for (size_t i = 0; i < currRenderObjects.size(); i++)
	{
		m_CommandList->SetGraphicsRootConstantBufferView(T1_OBJECT_CB, ObjectCBVritualAD);
		m_CommandList->DrawIndexedInstanced(currRenderObjectSubmesh[i]->numIndex, 1,
			currRenderObjectSubmesh[i]->indexOffset, currRenderObjectSubmesh[i]->vertexOffset, 0);
		ObjectCBVritualAD += ObjectStrideSize;
	}
}

void GraphicDX12::DrawDynamicMehs()
{
	m_PSOCon->SetPSOToCommnadList(m_CommandList.Get(),
		{ m_BackBufferFormat }, m_DepthStencilFormat, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
		to_string(DX12_RENDER_TYPE_NORMAL_MESH), "T1", "0", "0", "0",
		to_string(DX12_RENDER_TYPE_NORMAL_MESH),
		to_string(DX12_RENDER_TYPE_NORMAL_MESH));

	ID3D12DescriptorHeap* descriptorHeaps[] = { m_TextureBuffer->GetHeap() };
	m_CommandList->SetDescriptorHeaps(1, descriptorHeaps);

	auto matBuffer = m_Materials->GetBufferResource();
	m_CommandList->SetGraphicsRootShaderResourceView(T1_MATERIAL_SRV, matBuffer->GetGPUVirtualAddress());
	m_CommandList->SetGraphicsRootConstantBufferView(T1_PASS_CB, m_FrameResource->passCB->Resource()->GetGPUVirtualAddress());
	m_CommandList->SetGraphicsRootDescriptorTable(T1_TEXTURE_TABLE, m_TextureBuffer->GetHeap()->GetGPUDescriptorHandleForHeapStart());

	m_CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	auto ObjectCBVritualAD = m_FrameResource->meshObjectCB[DX12_RENDER_TYPE_DYNAMIC_MESH]->Resource()->GetGPUVirtualAddress();
	const UINT ObjectStrideSize = m_FrameResource->meshObjectCB[DX12_RENDER_TYPE_DYNAMIC_MESH]->GetElementByteSize();

	auto& currRenderObjects = m_RenderObjects[DX12_RENDER_TYPE_DYNAMIC_MESH];
	auto& currRenderObjectSubmesh = m_RenderObjectsSubmesh[DX12_RENDER_TYPE_DYNAMIC_MESH];

	for (size_t i = 0; i < currRenderObjects.size(); i++)
	{
		D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
		D3D12_INDEX_BUFFER_VIEW indexBufferView = {};
		indexBufferView.Format = DXGI_FORMAT_R32_UINT;

		vertexBufferView.BufferLocation = m_DynamicBuffers[currRenderObjects[i].dynamicBufferIndex]->dynamicVertexBuffer->Resource()->GetGPUVirtualAddress();
		vertexBufferView.SizeInBytes = m_DynamicBuffers[currRenderObjects[i].dynamicBufferIndex]->dynamicVertexBuffer->GetBufferSize();
		vertexBufferView.StrideInBytes = sizeof(Vertex);

		indexBufferView.BufferLocation = m_DynamicBuffers[currRenderObjects[i].dynamicBufferIndex]->dynamicIndexBuffer->Resource()->GetGPUVirtualAddress();
		indexBufferView.SizeInBytes = m_DynamicBuffers[currRenderObjects[i].dynamicBufferIndex]->dynamicIndexBuffer->GetBufferSize();

		m_CommandList->IASetVertexBuffers(0, 1, &vertexBufferView);
		m_CommandList->IASetIndexBuffer(&indexBufferView);

		m_CommandList->SetGraphicsRootConstantBufferView(T1_OBJECT_CB, ObjectCBVritualAD);
		m_CommandList->DrawIndexedInstanced(currRenderObjectSubmesh[i]->numIndex, 1,
			currRenderObjectSubmesh[i]->indexOffset, currRenderObjectSubmesh[i]->vertexOffset, 0);
		ObjectCBVritualAD += ObjectStrideSize;
	}
}

void GraphicDX12::DrawSkinnedMesh()
{
	m_PSOCon->SetPSOToCommnadList(m_CommandList.Get(),
		{ m_BackBufferFormat }, m_DepthStencilFormat, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
		to_string(DX12_RENDER_TYPE_SKINNED_MESH), "T1", "0", "0", "0",
		to_string(DX12_RENDER_TYPE_SKINNED_MESH),
		to_string(DX12_RENDER_TYPE_SKINNED_MESH));

	ID3D12DescriptorHeap* descriptorHeaps[] = { m_TextureBuffer->GetHeap() };
	m_CommandList->SetDescriptorHeaps(1, descriptorHeaps);

	auto matBuffer = m_Materials->GetBufferResource();
	m_CommandList->SetGraphicsRootShaderResourceView(T1_MATERIAL_SRV, matBuffer->GetGPUVirtualAddress());
	m_CommandList->SetGraphicsRootConstantBufferView(T1_PASS_CB, m_FrameResource->passCB->Resource()->GetGPUVirtualAddress());
	m_CommandList->SetGraphicsRootDescriptorTable(T1_TEXTURE_TABLE, m_TextureBuffer->GetHeap()->GetGPUDescriptorHandleForHeapStart());

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
	D3D12_INDEX_BUFFER_VIEW indexBufferView = {};
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;

	auto ObjectCBVritualAD = m_FrameResource->meshObjectCB[DX12_RENDER_TYPE_SKINNED_MESH]->Resource()->GetGPUVirtualAddress();
	const UINT ObjectStrideSize = m_FrameResource->meshObjectCB[DX12_RENDER_TYPE_SKINNED_MESH]->GetElementByteSize();

	auto AniBoneCBVritualAD = m_FrameResource->aniBoneMatBuffer->Resource()->GetGPUVirtualAddress();
	const UINT AniBoneStrideSize = m_FrameResource->aniBoneMatBuffer->GetElementByteSize();

	vertexBufferView.BufferLocation = m_SkinnedVertexBuffer->GetBufferResource()->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = m_SkinnedVertexBuffer->GetBufferSize();
	vertexBufferView.StrideInBytes = sizeof(SkinnedVertex);

	indexBufferView.BufferLocation = m_SkinnedIndexBuffer->GetBufferResource()->GetGPUVirtualAddress();
	indexBufferView.SizeInBytes = m_SkinnedIndexBuffer->GetBufferSize();

	m_CommandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	m_CommandList->IASetIndexBuffer(&indexBufferView);
	m_CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	auto& currRenderObjects = m_RenderObjects[DX12_RENDER_TYPE_SKINNED_MESH];
	auto& currRenderObjectSubmesh = m_RenderObjectsSubmesh[DX12_RENDER_TYPE_SKINNED_MESH];

	for (size_t i = 0; i < currRenderObjects.size(); i++)
	{
		m_CommandList->SetGraphicsRootConstantBufferView(T1_OBJECT_CB, ObjectCBVritualAD);

		if (currRenderObjects[i].aniBoneIndex != -1)
		{
			m_CommandList->SetGraphicsRootConstantBufferView(T1_ANIBONE_CB,
				AniBoneCBVritualAD + (currRenderObjects[i].aniBoneIndex * AniBoneStrideSize));
		}

		m_CommandList->DrawIndexedInstanced(currRenderObjectSubmesh[i]->numIndex, 1,
			currRenderObjectSubmesh[i]->indexOffset, currRenderObjectSubmesh[i]->vertexOffset, 0);
		ObjectCBVritualAD += ObjectStrideSize;
	}
}

void GraphicDX12::DrawPointObjects()
{
	if (m_NumRenderPointObjects)
	{
		m_PSOCon->SetPSOToCommnadList(m_CommandList.Get(),
			{ m_BackBufferFormat }, m_DepthStencilFormat, D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT,
			to_string(DX12_RENDER_TYPE_POINT), "P1", "0", "0", "0",
			to_string(DX12_RENDER_TYPE_POINT),
			to_string(DX12_RENDER_TYPE_POINT),
			to_string(DX12_RENDER_TYPE_POINT));

		ID3D12DescriptorHeap* descriptorHeaps[] = { m_TextureBuffer->GetHeap() };
		m_CommandList->SetDescriptorHeaps(1, descriptorHeaps);

		m_CommandList->SetGraphicsRootShaderResourceView(P1_OBJECT_SRV, m_FrameResource->pointCB->Resource()->GetGPUVirtualAddress());
		m_CommandList->SetGraphicsRootConstantBufferView(P1_PASS_CB, m_FrameResource->passCB->Resource()->GetGPUVirtualAddress());
		m_CommandList->SetGraphicsRootDescriptorTable(P1_TEXTURE_TABLE, m_TextureBuffer->GetHeap()->GetGPUDescriptorHandleForHeapStart());

		D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};

		vertexBufferView.BufferLocation = m_Box_Plane_Vertices->Resource()->GetGPUVirtualAddress();
		vertexBufferView.SizeInBytes = m_Box_Plane_Vertices->GetBufferSize();
		vertexBufferView.StrideInBytes = m_Box_Plane_Vertices->GetElementByteSize();

		m_CommandList->IASetVertexBuffers(0, 1, &vertexBufferView);
		m_CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

		m_CommandList->DrawInstanced(m_NumRenderPointObjects, 1, 0, 0);
	}
}

void GraphicDX12::DrawUIs()
{
	if (m_NumRenderUIs)
	{
		m_PSOCon->SetPSOToCommnadList(m_CommandList.Get(),
			{ m_BackBufferFormat }, m_DepthStencilFormat, D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT,
			to_string(DX12_RENDER_TYPE_UI), "UI", "0", "0", "0",
			to_string(DX12_RENDER_TYPE_UI),
			to_string(DX12_RENDER_TYPE_UI),
			to_string(DX12_RENDER_TYPE_UI));

		ID3D12DescriptorHeap* descriptorHeaps[] = { m_TextureBuffer->GetHeap() };
		m_CommandList->SetDescriptorHeaps(1, descriptorHeaps);

		m_CommandList->SetGraphicsRootConstantBufferView(UI_PASS_CB, m_FrameResource->passCB->Resource()->GetGPUVirtualAddress());
		m_CommandList->SetGraphicsRootConstantBufferView(UI_UIPASS_CB, m_FrameResource->uiPassCB->Resource()->GetGPUVirtualAddress());
		m_CommandList->SetGraphicsRootDescriptorTable(UI_TEXTURE_TABLE, m_TextureBuffer->GetHeap()->GetGPUDescriptorHandleForHeapStart());

		D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};

		vertexBufferView.BufferLocation = m_UIInfomation->Resource()->GetGPUVirtualAddress();
		vertexBufferView.SizeInBytes = m_UIInfomation->GetBufferSize();
		vertexBufferView.StrideInBytes = m_UIInfomation->GetElementByteSize();

		m_CommandList->IASetVertexBuffers(0, 1, &vertexBufferView);
		m_CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

		m_CommandList->DrawInstanced(m_NumRenderUIs, 1, 0, 0);
	}
}
