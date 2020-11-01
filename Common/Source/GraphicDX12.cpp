#include "GraphicDX12.h"
#include "cCamera.h"
#include "DX12/DX12DrawSetNormalMesh.h"
#include "DX12/DX12DrawSetSkinnedMesh.h"
#include "DX12/DX12DrawSetHeightField.h"
#include "DX12/DX12DrawSetPointBase.h"
#include "DX12/DX12DrawSetLight.h"
#include "DX12/DX12DrawSetUI.h"
#include "GraphicDO.h"
#include "BaseClass.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "MeshReplacer.h"

using namespace DirectX;
using namespace std;

GraphicDX12::GraphicDX12()
{
	m_ScissorRect = {};
	m_ScreenViewport = {};
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

	m_PSOCon = make_unique<DX12PSOController>(m_D3dDevice.Get());
	m_PSOCon->InitBase_Raster_Blend_Depth();
	m_NormalMeshSet = make_unique<DX12MeshSet<Vertex>>();
	m_SkinnedMeshSet = make_unique<DX12MeshSet<SkinnedVertex>>();
	m_HeightFieldMeshSet = make_unique<DX12MeshSet<float>>();
	m_PassCB = make_unique<DX12UploadBuffer<DX12PassConstants>>(m_D3dDevice.Get(), m_NumFrameResource, true);
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

	CGH::GO.graphic.aspectRatio = (float)m_ClientWidth / m_ClientHeight;
	CGH::GO.graphic.fovAngleX = 2 * atanf(CGH::GO.graphic.aspectRatio * tanf(CGH::GO.graphic.fovAngleY));

	m_BaseFrustum.near_Far.x = CGH::GO.graphic.perspectiveNearZ;
	m_BaseFrustum.near_Far.y = CGH::GO.graphic.perspectiveFarZ;

	m_BaseFrustum.rightNormal = physx::PxVec4(physx::PxQuat(CGH::GO.graphic.fovAngleX / 2, physx::PxVec3(0, 1, 0)).rotate(physx::PxVec3(1, 0, 0)), 0).getNormalized();
	m_BaseFrustum.leftNormal = m_BaseFrustum.rightNormal;
	m_BaseFrustum.leftNormal.x = -m_BaseFrustum.leftNormal.x;
	m_BaseFrustum.upNormal = physx::PxVec4(physx::PxQuat(CGH::GO.graphic.fovAngleY / 2, -physx::PxVec3(1, 0, 0)).rotate(physx::PxVec3(0, 1, 0)), 0).getNormalized();
	m_BaseFrustum.downNormal = m_BaseFrustum.upNormal;
	m_BaseFrustum.downNormal.y = -m_BaseFrustum.downNormal.y;

	XMMATRIX P = XMMatrixPerspectiveFovLH(CGH::GO.graphic.fovAngleY, CGH::GO.graphic.aspectRatio,
		CGH::GO.graphic.perspectiveNearZ, CGH::GO.graphic.perspectiveFarZ);
	XMMATRIX OrthoP = XMMatrixOrthographicOffCenterLH(static_cast<float>(m_ScissorRect.left), static_cast<float>(m_ScissorRect.right),
		static_cast<float>(m_ScissorRect.bottom), static_cast<float>(m_ScissorRect.top), D3D12_MIN_DEPTH, D3D12_MAX_DEPTH);
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
		return &m_NormalMeshSet->MS;
		break;
	case CGH::SKINNED_MESH:
		return &m_SkinnedMeshSet->MS;
		break;
	case CGH::HEIGHTFIELD_MESH:
		return &m_HeightFieldMeshSet->MS;
		break;
	case CGH::MESH_TYPE_COUNT:
	default:
		assert(false);
		return nullptr;
		break;
	}
}

bool GraphicDX12::CreateMesh(const std::string& meshName, MeshObject& meshinfo, CGH::MESH_TYPE type, unsigned int numVertices, const void* vertices, const std::vector<unsigned int>& indices)
{
	switch (type)
	{
	case CGH::NORMAL_MESH:
		if (m_NormalMeshSet->MS.find(meshName) != m_NormalMeshSet->MS.end())
		{
			return false;
		}
		break;
	case CGH::SKINNED_MESH:
		if (m_SkinnedMeshSet->MS.find(meshName) != m_SkinnedMeshSet->MS.end())
		{
			return false;
		}
		break;
	case CGH::HEIGHTFIELD_MESH:
		if (m_HeightFieldMeshSet->MS.find(meshName) != m_HeightFieldMeshSet->MS.end())
		{
			return false;
		}
		break;
	case CGH::MESH_TYPE_COUNT:
	default:
		assert(false);
		break;
	}

	bool result = false;
	ComPtr<ID3D12CommandAllocator>		allocator;
	ComPtr<ID3D12GraphicsCommandList>	commandList;

	ThrowIfFailed(m_D3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(allocator.GetAddressOf())));
	ThrowIfFailed(m_D3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		allocator.Get(), nullptr, IID_PPV_ARGS(commandList.GetAddressOf())));

	DX12MeshSetResult prevResource;

	switch (type)
	{
	case CGH::NORMAL_MESH:
		prevResource = m_NormalMeshSet->AddMesh(m_D3dDevice.Get(), commandList.Get(), meshName, meshinfo, numVertices, reinterpret_cast<const Vertex*>(vertices), indices);
		break;
	case CGH::SKINNED_MESH:
		prevResource = m_SkinnedMeshSet->AddMesh(m_D3dDevice.Get(), commandList.Get(), meshName, meshinfo, numVertices, reinterpret_cast<const SkinnedVertex*>(vertices), indices);
		break;
	case CGH::HEIGHTFIELD_MESH:
		prevResource = m_HeightFieldMeshSet->AddMesh(m_D3dDevice.Get(), commandList.Get(), meshName, meshinfo, numVertices, reinterpret_cast<const float*>(vertices), indices);
		break;
	case CGH::MESH_TYPE_COUNT:
	default:
		assert(false);
		break;
	}

	commandList->Close();

	ID3D12CommandList* cmdLists[] = { commandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

	FlushCommandQueue();
	ThrowIfFailed(allocator->Reset());

	switch (type)
	{
	case CGH::NORMAL_MESH:
		m_NormalMeshSet->ClearUploadBuffer();
		break;
	case CGH::SKINNED_MESH:
		m_SkinnedMeshSet->ClearUploadBuffer();
		break;
	case CGH::HEIGHTFIELD_MESH:
		m_HeightFieldMeshSet->ClearUploadBuffer();
		break;
	}

	return result;
}

bool GraphicDX12::CreateMaterials(const std::vector<std::string>& materialNames, const std::vector<Material>& materials)
{
	for (auto& it : materialNames)
	{
		if (m_Materials->GetIndex(it) == -1)
		{
			return false;
		}
	}

	ComPtr<ID3D12CommandAllocator>		allocator;
	ComPtr<ID3D12GraphicsCommandList>	commandList;

	ThrowIfFailed(m_D3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(allocator.GetAddressOf())));
	ThrowIfFailed(m_D3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		allocator.Get(), nullptr, IID_PPV_ARGS(commandList.GetAddressOf())));

	auto result = m_Materials->IndexedAddData(m_D3dDevice.Get(), commandList.Get(), 
		CGH::SizeTTransUINT(materials.size()), materials.data(), materialNames);

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

	result = m_NormalMeshSet->EditMesh(m_D3dDevice.Get(), commandList.Get(), meshName, vertices);

	commandList->Close();
	ID3D12CommandList* cmdLists[] = { commandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

	FlushCommandQueue();
	ThrowIfFailed(allocator->Reset());

	m_NormalMeshSet->ClearUploadBuffer();

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

int GraphicDX12::GetTextureIndex(const std::wstring& group, const std::string& textureName)
{
	return m_TextureBuffers[group]->GetTextureIndex(textureName);
}

void GraphicDX12::ReComputeHeightField(const std::string& name, physx::PxVec3 scale)
{
	ComPtr<ID3D12CommandAllocator>		allocator;
	ComPtr<ID3D12GraphicsCommandList>	commandList;

	ThrowIfFailed(m_D3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(allocator.GetAddressOf())));
	ThrowIfFailed(m_D3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		allocator.Get(), nullptr, IID_PPV_ARGS(commandList.GetAddressOf())));

	m_HeightFieldMeshDrawSet->ReComputeHeightField(name, scale, m_D3dDevice.Get(), commandList.Get());

	commandList->Close();

	ID3D12CommandList* cmdLists[] = { commandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

	FlushCommandQueue();
	ThrowIfFailed(allocator->Reset());
}

void GraphicDX12::LoadTextureFromFolder(const std::vector<std::wstring>& targetTextureFolders)
{
	vector<vector<wstring>> folders(targetTextureFolders.size());
	vector<string> files;
	for (size_t i = 0; i < targetTextureFolders.size(); i++)
	{
		SearchAllFolderFromFolder(targetTextureFolders[i], folders[i]);
	}

	for (size_t i = 0; i < targetTextureFolders.size(); i++)
	{
		for (auto& it : folders[i])
		{
			SearchAllFileFromFolder(targetTextureFolders[i] + L"//" + it, true, files);
			unordered_map<string, wstring> texturePaths;

			for (auto& it2 : files)
			{
				string extension;
				string fileName = GetFileNameFromPath(it2, extension);
				wstring temp(it2.begin(), it2.end());

				if (CheckFileExtension(extension) == EXTENSIONTYPE::EXE_TEXTURE)
				{
					texturePaths[fileName] = temp;
				}
			}

			m_TextureBuffers[it] = make_unique<DX12TextureBuffer>(m_D3dDevice.Get(), texturePaths.size());

			auto currBuffer = m_TextureBuffers[it].get();
			currBuffer->Begin(m_D3dDevice.Get());

			for (auto& it2 : texturePaths)
			{
				currBuffer->AddTexture(m_D3dDevice.Get(),
					m_CommandQueue.Get(), it2.second);
			}

			files.clear();
		}
	}

	auto flushFunc = bind(&GraphicDX12::FlushCommandQueue, this);

	for (auto& it : m_TextureBuffers)
	{
		it.second->End(m_CommandQueue.Get(), flushFunc);
	}
}

void GraphicDX12::LoadMeshAndMaterialFromFolder(const std::vector<std::wstring>& targetMeshFolders)
{
	ThrowIfFailed(m_CommandList->Reset(m_DirectCmdListAlloc.Get(), nullptr));

	vector<string> files;

	for (auto& it : targetMeshFolders)
	{
		SearchAllFileFromFolder(it, true, files);
	}

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

	Assimp::Importer importer;
	MeshReplacer replacer;

	for (auto& it : files)
	{
		bool isSkinnedMesh = true;
		MeshObject meshObject;
		string extension;
		string fileName = GetFileNameFromPath(it, extension);

		const aiScene* scene = importer.ReadFile(it,
			aiProcess_Triangulate | 
			aiProcess_ConvertToLeftHanded);

		wstring temp(it.begin(), it.end());
		indices.clear();

		UINT baseVertexOffset = 0;
		UINT baseIndexOffset = 0;
		meshObject.primitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		skinnedVertices.clear();
		vertices.clear();

		baseVertexOffset = static_cast<UINT>(combinedSkinnedVertex.size());
		baseIndexOffset = static_cast<UINT>(combinedSkinnedIndices.size());

		replacer.Replace(scene, skinnedVertices, indices, subsets, mats, skinnedData);

		for (auto& it2 : subsets)
		{
			UINT faceCount = 0;

			if (it2.primitiveType == aiPrimitiveType_TRIANGLE)
			{
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
			else
			{
				for (auto& materalIndex : it2.materialIndexCount)
				{
					faceCount += materalIndex.second;
				}
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
			material.specular = it2.specular;
			material.specularExponent = it2.specularExponent;
			material.emissive = it2.emissive;
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
			assert(m_SkinnedDatas.find(fileName) == m_SkinnedDatas.end());
			m_SkinnedDatas.insert({ fileName, skinnedData });
			skinnedMeshs.push_back(meshObject);
			skinnedMeshNames.push_back(fileName);
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

		importer.FreeScene();
	}

	m_Materials = make_unique<DX12IndexManagementBuffer<Material>>(m_D3dDevice.Get(),
		m_CommandList.Get(), matNames, matDatas);

	m_NormalMeshSet->AddMeshs(m_D3dDevice.Get(), m_CommandList.Get(),
		normalMeshNames, normalMeshs, combinedVertex, combinedIndices);

	m_SkinnedMeshSet->AddMeshs(m_D3dDevice.Get(), m_CommandList.Get(),
		skinnedMeshNames, skinnedMeshs, combinedSkinnedVertex, combinedSkinnedIndices);

	ThrowIfFailed(m_CommandList->Close());
	ID3D12CommandList* cmdsLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
	FlushCommandQueue();

	ThrowIfFailed(m_DirectCmdListAlloc->Reset());

	m_Materials->ClearUploadBuffer();
	m_NormalMeshSet->ClearUploadBuffer();
	m_SkinnedMeshSet->ClearUploadBuffer();
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
	BuildDrawSets();

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

		if (eventHandle)
		{
			WaitForSingleObject(eventHandle, INFINITE);
			CloseHandle(eventHandle);
		}
		else
		{
			assert(false);
		}
	}
}

void GraphicDX12::Update(const CGHScene& scene, float delta)
{
	if (m_CurrCamera)
	{
		m_ViewMatrix = *m_CurrCamera->GetViewMatrix();
	}

	UpdateMainPassCB(delta);
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

	m_Swap->RenderBegin(m_CommandList.Get(), Colors::Gray);

	m_NormalMeshDrawSet->Draw(m_CommandList.Get());
	m_SkinnedMeshDrawSet->Draw(m_CommandList.Get());
	m_PointBaseDrawSet->Draw(m_CommandList.Get());
	m_HeightFieldMeshDrawSet->Draw(m_CommandList.Get());

	m_LightDrawSet->Draw(m_CommandList.Get());

	m_Swap->ClearDepth(m_CommandList.Get());
	m_UIDrawSet->Draw(m_CommandList.Get());

	m_FontManager->RenderCommandWrite(m_CommandList.Get(), m_ReservedFonts);

	m_Swap->RenderEnd(m_CommandList.Get());

	FlushCommandQueue();

	ThrowIfFailed(m_CommandList->Close());
	ID3D12CommandList* cmdsLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
	m_FontManager->Commit(m_CommandQueue.Get());

	m_Swap->Present();

	FlushCommandQueue();

	DX12DrawSet::AllDrawsFrameCountAndClearWork();
	m_CurrFrame = (m_CurrFrame + 1) % m_NumFrameResource;
}

void GraphicDX12::BuildDrawSets()
{
	DX12DrawSet::SetBaseResource(m_PassCB->Resource(), m_Materials.get(), m_Swap.get());
	DX12MeshComputeCulling::BaseSetting(m_D3dDevice.Get(), m_PSOCon.get(), &m_BaseFrustum);
	std::vector<DXGI_FORMAT> rtv;
	m_Swap->GetRenderTargetFormats(rtv);

	m_NormalMeshDrawSet = std::make_unique<DX12DrawSetNormalMesh>(1, m_PSOCon.get(),
		m_TextureBuffers[L"MESH"].get(), rtv, m_DepthStencilFormat, *m_NormalMeshSet);

	m_SkinnedMeshDrawSet = std::make_unique<DX12DrawSetSkinnedMesh>(1, m_PSOCon.get(),
		m_TextureBuffers[L"MESH"].get(), rtv, m_DepthStencilFormat, *m_SkinnedMeshSet);

	m_HeightFieldMeshDrawSet = std::make_unique<DX12DrawSetHeightField>(1, m_PSOCon.get(),
		m_TextureBuffers[L"HEIGHT"].get(), rtv, m_DepthStencilFormat, *m_HeightFieldMeshSet);

	m_PointBaseDrawSet = std::make_unique<DX12DrawSetPointBase>(1, m_PSOCon.get(),
		m_TextureBuffers[L"BASE"].get(), rtv, m_DepthStencilFormat);

	m_LightDrawSet = std::make_unique<DX12DrawSetLight>(1, m_PSOCon.get(),
		m_TextureBuffers[L"BASE"].get(), rtv, m_DepthStencilFormat);

	m_UIDrawSet = std::make_unique<DX12DrawSetUI>(1, m_PSOCon.get(),
		m_TextureBuffers[L"UI"].get(), rtv, m_DepthStencilFormat);

	m_NormalMeshDrawSet->Init(m_D3dDevice.Get());
	m_SkinnedMeshDrawSet->Init(m_D3dDevice.Get());
	m_HeightFieldMeshDrawSet->Init(m_D3dDevice.Get());
	m_PointBaseDrawSet->Init(m_D3dDevice.Get());
	m_LightDrawSet->Init(m_D3dDevice.Get());
	m_UIDrawSet->Init(m_D3dDevice.Get());
}

void GraphicDX12::BuildDepthStencilAndBlendsAndRasterizer()
{

}

void GraphicDX12::UpdateMainPassCB(float delta)
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
	DX12PassConstants mainPass;

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
	mainPass.eyePosW = m_CurrCamera->GetEyePos();
	mainPass.ambientLight = { 0.25f, 0.25f, 0.35f, 1.0f };

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

	m_BaseFrustum.viewMat = mainPass.view;
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
		case RENDER_HEIGHT_FIELD:
		{
			m_HeightFieldMeshDrawSet->ReserveRender(it);
		}
		break;
		case RENDER_BOX:
		case RENDER_PLANE:
		case RENDER_2DPLANE:
		{
			m_PointBaseDrawSet->ReserveRender(it);
		}
		break;
		case RENDER_LIGHT:
		{
			m_LightDrawSet->ReserveRender(it);
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