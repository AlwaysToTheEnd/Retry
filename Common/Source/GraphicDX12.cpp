#include "GraphicDX12.h"
#include "cCamera.h"
#include "DX12/DX12TextureBuffer.h"
#include "DX12/DX12DrawSetNormalMesh.h"
#include "DX12/DX12DrawSetSkinnedMesh.h"
#include "DX12/DX12DrawSetPointBase.h"
#include "DX12/DX12DrawSetUI.h"
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

#if defined(DEBUG)||defined(_DEBUG)
	{
		ComPtr<ID3D12Debug> debugController;
		ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(debugController.GetAddressOf())));
		debugController->EnableDebugLayer();
	}
#endif

	HRESULT hr = S_OK;

	hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(m_D3dDevice.GetAddressOf()));

	m_Swap = std::make_unique<DX12SwapChain>();
	m_Swap->CreateDXGIFactory(m_D3dDevice.GetAddressOf());

	ThrowIfFailed(m_D3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(m_Fence.GetAddressOf())));

	CreateCommandObject();

	m_Swap->CreateSwapChain(m_MainWndHandle, m_CommandQueue.Get(),
		m_BackBufferFormat, m_DepthStencilFormat, m_ClientWidth, m_ClientHeight, 2);

	OnResize();

	ThrowIfFailed(m_DirectCmdListAlloc->Reset());

	m_PSOCon = make_unique<PSOController>(m_D3dDevice.Get());
	m_PSOCon->InitBase_Raster_Blend_Depth();
	m_PassCB = make_unique<DX12UploadBuffer<PassConstants>>(m_D3dDevice.Get(), m_NumFrameResource, true);
	m_CmdListAllocs.resize(m_NumFrameResource);

	for (UINT i = 0; i < m_NumFrameResource; i++)
	{
		ThrowIfFailed(m_D3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(m_CmdListAllocs[i].GetAddressOf())));
	}

	return true;
}

void GraphicDX12::CreateCommandObject()
{
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

void GraphicDX12::OnResize()
{
	assert(m_D3dDevice);
	assert(m_DirectCmdListAlloc);

	FlushCommandQueue();

	ThrowIfFailed(m_CommandList->Reset(m_DirectCmdListAlloc.Get(), nullptr));

	m_Swap->ReSize(m_CommandList.Get(), m_ClientWidth, m_ClientHeight);

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

const std::unordered_map<std::string, MeshObject>* GraphicDX12::GetMeshDataMap(CGH::MESH_TYPE type)
{
	switch (type)
	{
	case CGH::NORMAL_MESH:
		return m_NormalMeshDrawSet->GetMeshs();
		break;
	case CGH::SKINNED_MESH:
		return m_SkinnedMeshDrawSet->GetMeshs();
		break;
	case CGH::MESH_TYPE_COUNT:
	default:
		return nullptr;
		assert(false);
		break;
	}
}

bool GraphicDX12::CreateMesh(const std::string& meshName, MeshObject& meshinfo,
	const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices)
{
	bool result = false;
	ComPtr<ID3D12CommandAllocator>		allocator;
	ComPtr<ID3D12GraphicsCommandList>	commandList;

	ThrowIfFailed(m_D3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(allocator.GetAddressOf())));
	ThrowIfFailed(m_D3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		allocator.Get(), nullptr, IID_PPV_ARGS(commandList.GetAddressOf())));

	result = m_NormalMeshDrawSet->AddMesh(m_D3dDevice.Get(), commandList.Get(), meshName, meshinfo, vertices, indices);

	commandList->Close();

	ID3D12CommandList* cmdLists[] = { commandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

	FlushCommandQueue();
	ThrowIfFailed(allocator->Reset());

	m_NormalMeshDrawSet->UploadBuffersClear();

	return result;
}

bool GraphicDX12::CreateMaterials(const std::vector<std::string>& materialNames, const std::vector<Material>& materials)
{
	ComPtr<ID3D12CommandAllocator>		allocator;
	ComPtr<ID3D12GraphicsCommandList>	commandList;

	ThrowIfFailed(m_D3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(allocator.GetAddressOf())));
	ThrowIfFailed(m_D3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		allocator.Get(), nullptr, IID_PPV_ARGS(commandList.GetAddressOf())));

	auto result = m_Materials->IndexedAddData(m_D3dDevice.Get(), commandList.Get(), materials.size(), materials.data(), materialNames);

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
	ComPtr<ID3D12CommandAllocator>		allocator;
	ComPtr<ID3D12GraphicsCommandList>	commandList;

	ThrowIfFailed(m_D3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(allocator.GetAddressOf())));
	ThrowIfFailed(m_D3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		allocator.Get(), nullptr, IID_PPV_ARGS(commandList.GetAddressOf())));

	result = m_NormalMeshDrawSet->EditMesh(m_D3dDevice.Get(), commandList.Get(), meshName, vertices);

	commandList->Close();
	ID3D12CommandList* cmdLists[] = { commandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

	FlushCommandQueue();
	ThrowIfFailed(allocator->Reset());

	m_NormalMeshDrawSet->UploadBuffersClear();

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
			auto temp= make_unique<DynamicBuffer>(m_D3dDevice.Get(), i, vertexNum, indexNum);

			m_DynamicBuffers[i] = std::move(temp);

			addedDyanamic = m_DynamicBuffers[i].get();
			break;
		}
	}

	if (addedDyanamic == nullptr)
	{
		auto newDynamic = new DynamicBuffer(m_D3dDevice.Get(), m_DynamicBuffers.size(), vertexNum, indexNum);
		m_DynamicBuffers.push_back(std::unique_ptr<DynamicBuffer>(newDynamic));
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
	m_TextureBuffer = make_unique<DX12TextureBuffer>(m_D3dDevice.Get(), numTexturePath);

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

	std::vector<std::string> skinnedMeshNames;
	std::vector<std::string> normalMeshNames;

	std::vector<MeshObject> skinnedMeshs;
	std::vector<MeshObject> normalMeshs;

	for (auto& it : files)
	{
		bool isSkinnedMesh = true;
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
				normalMeshs.push_back(meshObject);
				normalMeshNames.push_back(fileName);
				isSkinnedMesh = false;
			}
			else
			{
				m_SkinnedDatas.insert({ fileName, skinnedData });
				skinnedMeshs.push_back(meshObject);
				skinnedMeshNames.push_back(fileName);
			}
		}

		if (isSkinnedMesh)
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

	m_Materials = make_unique<DX12IndexManagementBuffer<Material>>(m_D3dDevice.Get(),
		m_CommandList.Get(), matNames, matDatas);

	BuildDrawSets();

	m_NormalMeshDrawSet->AddMeshs(m_D3dDevice.Get(), m_CommandList.Get(),
		normalMeshNames, normalMeshs, combinedVertex, combinedIndices);

	m_SkinnedMeshDrawSet->AddMeshs(m_D3dDevice.Get(), m_CommandList.Get(),
		skinnedMeshNames, skinnedMeshs, combinedSkinnedVertex, combinedSkinnedIndices);

	ThrowIfFailed(m_CommandList->Close());
	ID3D12CommandList* cmdsLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
	FlushCommandQueue();

	ThrowIfFailed(m_DirectCmdListAlloc->Reset());

	m_Materials->ClearUploadBuffer();
	m_NormalMeshDrawSet->UploadBuffersClear();
	m_SkinnedMeshDrawSet->UploadBuffersClear();
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

	BuildDepthStencilAndBlendsAndRasterizer();

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
	m_SkinnedMeshDrawSet->UpdateAniBoneCB(m_ReservedAniBones);
}

void GraphicDX12::Draw()
{
	auto cmdListAlloc = m_CmdListAllocs[m_CurrFrame].Get();

	ThrowIfFailed(cmdListAlloc->Reset());

	ThrowIfFailed(m_CommandList->Reset(cmdListAlloc, nullptr));

	m_CommandList->RSSetViewports(1, &m_ScreenViewport);
	m_CommandList->RSSetScissorRects(1, &m_ScissorRect);

	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_Swap->CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	m_CommandList->OMSetRenderTargets(1, &m_Swap->CurrentBackBufferView(), true, &m_Swap->DepthStencilView());

	m_CommandList->ClearRenderTargetView(m_Swap->CurrentBackBufferView(), Colors::Gray, 0, nullptr);
	m_CommandList->ClearDepthStencilView(m_Swap->DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	m_NormalMeshDrawSet->Draw(m_CommandList.Get());
	m_SkinnedMeshDrawSet->Draw(m_CommandList.Get());
	m_PointBaseDrawSet->Draw(m_CommandList.Get());
	m_UIDrawSet->Draw(m_CommandList.Get());

	m_FontManager->RenderCommandWrite(m_CommandList.Get(), m_ReservedFonts);

	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_Swap->CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
	ThrowIfFailed(m_CommandList->Close());
	ID3D12CommandList* cmdsLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
	m_FontManager->Commit(m_CommandQueue.Get());

	m_Swap->Presnet();

	FlushCommandQueue();

	m_CurrFrame = (m_CurrFrame + 1) % m_NumFrameResource;

	m_NormalMeshDrawSet->UpdateFrameCount();
	m_SkinnedMeshDrawSet->UpdateFrameCount();
	m_PointBaseDrawSet->UpdateFrameCount();
	m_UIDrawSet->UpdateFrameCount();
}


void GraphicDX12::BuildDrawSets()
{
	m_NormalMeshDrawSet = std::make_unique<DX12DrawSetNormalMesh>(1);
	m_SkinnedMeshDrawSet = std::make_unique<DX12DrawSetSkinnedMesh>(1);
	m_PointBaseDrawSet = std::make_unique<DX12DrawSetPointBase>(1);
	m_UIDrawSet = std::make_unique<DX12DrawSetUI>(1);

	m_NormalMeshDrawSet->Init(m_D3dDevice.Get(), m_PSOCon.get(), m_BackBufferFormat, m_DepthStencilFormat,
		m_TextureBuffer.get(), m_Materials.get(), m_PassCB->Resource());

	m_SkinnedMeshDrawSet->Init(m_D3dDevice.Get(), m_PSOCon.get(), m_BackBufferFormat, m_DepthStencilFormat,
		m_TextureBuffer.get(), m_Materials.get(), m_PassCB->Resource());

	m_PointBaseDrawSet->Init(m_D3dDevice.Get(), m_PSOCon.get(), m_BackBufferFormat, m_DepthStencilFormat,
		m_TextureBuffer.get(), m_Materials.get(), m_PassCB->Resource());

	m_UIDrawSet->Init(m_D3dDevice.Get(), m_PSOCon.get(), m_BackBufferFormat, m_DepthStencilFormat,
		m_TextureBuffer.get(), m_Materials.get(), m_PassCB->Resource());
}

void GraphicDX12::BuildDepthStencilAndBlendsAndRasterizer()
{
	
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
	PassConstants mainPass;

	mainPass.view = view.getTranspose();
	mainPass.invView = invView.getTranspose();
	mainPass.proj = proj.getTranspose();
	mainPass.invProj = invProj.getTranspose();
	mainPass.viewProj = viewProj.getTranspose();
	mainPass.invViewProj = invViewProj.getTranspose();
	mainPass.orthoMatrix = m_OrthoProjectionMat.getTranspose();
	mainPass.renderTargetSize = physx::PxVec2((float)m_ClientWidth, (float)m_ClientHeight);
	mainPass.invRenderTargetSize = physx::PxVec2(1.0f / m_ClientWidth, 1.0f / m_ClientHeight);
	mainPass.samplerIndex = CGH::GO.graphic.samplerIndex;

	mainPass.ambientLight = { 0.25f, 0.25f, 0.35f, 1.0f };
	//m_MainPassCB.Lights[0].direction = { 0.57735f, -0.57735f, 0.57735f };
	//m_MainPassCB.Lights[0].strength = { 0.9f, 0.9f, 0.9f };
	//m_MainPassCB.Lights[1].direction = { -0.57735f, -0.57735f, 0.57735f };
	//m_MainPassCB.Lights[1].strength = { 0.5f, 0.5f, 0.5f };
	//m_MainPassCB.Lights[2].direction = { 0.0f, -0.707f, -0.707f };
	//m_MainPassCB.Lights[2].strength = { 0.2f, 0.2f, 0.2f };

	m_PassCB->CopyData(0, mainPass);
	m_UIDrawSet->UpdateUIPassCB(CGH::GO.ui);

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
	for (auto& it : m_ReservedRenderInfos)
	{
		switch (it.type)
		{
		case RENDER_MESH:
		{
			m_NormalMeshDrawSet->ReserveRender(it);
		}
		break;
		case RENDER_SKIN:
		{
			m_SkinnedMeshDrawSet->ReserveRender(it);
		}
		break;
		case RENDER_DYNAMIC:
		{
			ObjectConstants object;
			object.world = it.world;
			object.scale = it.scale;
			object.PrevAniBone = atoi(it.meshOrTextureName.c_str());

			const MeshObject& mesh = m_DynamicBuffers[object.PrevAniBone]->dynamicBufferInfo->meshObject;
			for (auto& it2 : mesh.subs)
			{
				/*object.diffuseMapIndex = m_TextureBuffer->GetTextureIndex(it2.second.diffuseMap);
				object.normalMapIndex = m_TextureBuffer->GetTextureIndex(it2.second.normalMap);
				object.materialIndex = m_Materials->GetIndex(it2.second.material);
				m_RenderObjects[DX12_RENDER_TYPE_DYNAMIC_MESH].push_back(object);
				m_RenderObjectsSubmesh[DX12_RENDER_TYPE_DYNAMIC_MESH].push_back(&it2.second);*/
			}
		}
		break;
		case RENDER_BOX:
		case RENDER_PLANE:
		case RENDER_2DPLANE:
		{
			m_PointBaseDrawSet->ReserveRender(it);
		}
		break;
		case RENDER_UI:
		{
			m_UIDrawSet->ReserveRender(it);
		}
		break;
		default:
			assert(false);
			break;
		}
	}
}


//void GraphicDX12::DrawDynamicMehs()
//{
//	m_PSOCon->SetPSOToCommnadList(m_CommandList.Get(),
//		{ m_BackBufferFormat }, m_DepthStencilFormat, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
//		"normal", "normal", "0", "0", "0",
//		"normal", "normal");
//	enum
//	{
//		PASS_CB,
//		TEXTURE_TABLE,
//		MATERIAL_SRV,
//		OBJECT_CB,
//		ROOT_COUNT
//	};
//
//	ID3D12DescriptorHeap* descriptorHeaps[] = { m_TextureBuffer->GetHeap() };
//	m_CommandList->SetDescriptorHeaps(1, descriptorHeaps);
//
//	auto matBuffer = m_Materials->GetBufferResource();
//	m_CommandList->SetGraphicsRootConstantBufferView(PASS_CB, m_FrameResource->passCB->Resource()->GetGPUVirtualAddress());
//	m_CommandList->SetGraphicsRootShaderResourceView(MATERIAL_SRV, matBuffer->GetGPUVirtualAddress());
//	m_CommandList->SetGraphicsRootDescriptorTable(TEXTURE_TABLE, m_TextureBuffer->GetHeap()->GetGPUDescriptorHandleForHeapStart());
//
//	m_CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//
//	auto ObjectCBVritualAD = m_FrameResource->meshObjectCB[DX12_RENDER_TYPE_DYNAMIC_MESH]->Resource()->GetGPUVirtualAddress();
//	const UINT ObjectStrideSize = m_FrameResource->meshObjectCB[DX12_RENDER_TYPE_DYNAMIC_MESH]->GetElementByteSize();
//
//	auto& currRenderObjects = m_RenderObjects[DX12_RENDER_TYPE_DYNAMIC_MESH];
//	auto& currRenderObjectSubmesh = m_RenderObjectsSubmesh[DX12_RENDER_TYPE_DYNAMIC_MESH];
//
//	for (size_t i = 0; i < currRenderObjects.size(); i++)
//	{
//		D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
//		D3D12_INDEX_BUFFER_VIEW indexBufferView = {};
//		indexBufferView.Format = DXGI_FORMAT_R32_UINT;
//
//		vertexBufferView.BufferLocation = m_DynamicBuffers[currRenderObjects[i].PrevAniBone]->dynamicVertexBuffer->Resource()->GetGPUVirtualAddress();
//		vertexBufferView.SizeInBytes = m_DynamicBuffers[currRenderObjects[i].PrevAniBone]->dynamicVertexBuffer->GetBufferSize();
//		vertexBufferView.StrideInBytes = sizeof(Vertex);
//
//		indexBufferView.BufferLocation = m_DynamicBuffers[currRenderObjects[i].PrevAniBone]->dynamicIndexBuffer->Resource()->GetGPUVirtualAddress();
//		indexBufferView.SizeInBytes = m_DynamicBuffers[currRenderObjects[i].PrevAniBone]->dynamicIndexBuffer->GetBufferSize();
//
//		m_CommandList->IASetVertexBuffers(0, 1, &vertexBufferView);
//		m_CommandList->IASetIndexBuffer(&indexBufferView);
//
//		m_CommandList->SetGraphicsRootConstantBufferView(OBJECT_CB, ObjectCBVritualAD);
//		m_CommandList->DrawIndexedInstanced(currRenderObjectSubmesh[i]->numIndex, 1,
//			currRenderObjectSubmesh[i]->indexOffset, currRenderObjectSubmesh[i]->vertexOffset, 0);
//		ObjectCBVritualAD += ObjectStrideSize;
//	}
//}
