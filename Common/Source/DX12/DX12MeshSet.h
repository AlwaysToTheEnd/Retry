#pragma once
#include "GraphicBase.h"
#include "DX12DefaultBuffer.h"
#include "DX12UploadBuffer.h"
#include "DX12TextureBuffer.h"

struct DX12MeshSetResult
{
	Microsoft::WRL::ComPtr<ID3D12Resource> VBResult;
	Microsoft::WRL::ComPtr<ID3D12Resource> IBResult;
};

template <typename Vertex_T>
struct DX12MeshSet
{
	DX12MeshSet()=default;
	~DX12MeshSet()=default;

	DX12MeshSetResult			AddMesh(ID3D12Device* device, ID3D12GraphicsCommandList* commandList,
										const std::string& name, MeshObject& mesh,
										const std::vector<Vertex_T>& vertices, const std::vector<UINT>& indices);
	DX12MeshSetResult			AddMesh(ID3D12Device* device, ID3D12GraphicsCommandList* commandList,
										const std::string& name, MeshObject& mesh,
										unsigned int numVertex, const Vertex_T* vertices, const std::vector<UINT>& indices);
	DX12MeshSetResult			AddMeshs(ID3D12Device* device, ID3D12GraphicsCommandList* commandList,
										const std::vector<std::string>& meshNames, const std::vector<MeshObject>& meshs,
										const std::vector<Vertex_T>& vertices, const std::vector<UINT>& indices);

	bool						EditMesh(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, const std::string& meshName, const std::vector<Vertex>& vertices);
	void						ClearUploadBuffer();

	D3D12_VERTEX_BUFFER_VIEW	GetVertexBufferView() const;
	D3D12_INDEX_BUFFER_VIEW		GetIndexBufferView() const;

	std::unique_ptr<DX12TextureBuffer>				TB;
	std::unordered_map<std::string, MeshObject>		MS;
	std::unique_ptr<DX12DefaultBuffer<Vertex_T>>	VB;
	std::unique_ptr<DX12DefaultBuffer<UINT>>		IB;
};

template<typename Vertex_T>
inline DX12MeshSetResult DX12MeshSet<Vertex_T>::AddMesh(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, const std::string& name, MeshObject& mesh, const std::vector<Vertex_T>& vertices, const std::vector<UINT>& indices)
{
	DX12MeshSetResult result;

	if (VB == nullptr)
	{
		VB = std::make_unique<DX12DefaultBuffer<Vertex_T>>(device, commandList, vertices);
		IB = std::make_unique<DX12DefaultBuffer<UINT>>(device, commandList, indices);
	}
	else
	{
		UINT baseVertexLocation = 0;
		UINT baseIndexLocation = 0;
		UINT numVertices = 0;
		UINT numIndices = 0;

		baseVertexLocation = VB->GetNumDatas();
		baseIndexLocation = IB->GetNumDatas();
		numVertices = vertices.size();
		numIndices = indices.size();

		for (auto& it : mesh.subs)
		{
			it.second.vertexOffset += baseVertexLocation;
			it.second.indexOffset += baseIndexLocation;
		}

		result.VBResult = VB->AddData(device, commandList, numVertices, vertices.data());
		result.IBResult = IB->AddData(device, commandList, numIndices, indices.data());
	}

	MS.insert({ name, mesh });

	return result;
}

template<typename Vertex_T>
inline DX12MeshSetResult DX12MeshSet<Vertex_T>::AddMesh(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, const std::string& name, MeshObject& mesh, unsigned int numVertex, const Vertex_T* vertices, const std::vector<UINT>& indices)
{
	DX12MeshSetResult result;

	if (VB == nullptr)
	{
		VB = std::make_unique<DX12DefaultBuffer<Vertex_T>>(device, commandList, numVertex, vertices);
		IB = std::make_unique<DX12DefaultBuffer<UINT>>(device, commandList, indices);
	}
	else
	{
		UINT baseVertexLocation = 0;
		UINT baseIndexLocation = 0;
		UINT numIndices = 0;
		
		baseVertexLocation = VB->GetNumDatas();
		baseIndexLocation = IB->GetNumDatas();
		numIndices = CGH::SizeTTransUINT(indices.size());

		for (auto& it : mesh.subs)
		{
			it.second.vertexOffset += baseVertexLocation;
			it.second.indexOffset += baseIndexLocation;
		}

		result.VBResult = VB->AddData(device, commandList, numVertex, vertices);
		result.IBResult = IB->AddData(device, commandList, numIndices, indices.data());
	}

	MS.insert({ name, mesh });

	return result;
}

template<typename Vertex_T>
inline DX12MeshSetResult DX12MeshSet<Vertex_T>::AddMeshs(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, const std::vector<std::string>& meshNames, const std::vector<MeshObject>& meshs, const std::vector<Vertex_T>& vertices, const std::vector<UINT>& indices)
{
	DX12MeshSetResult result;

	if (VB == nullptr)
	{
		VB = std::make_unique<DX12DefaultBuffer<Vertex_T>>(device, commandList, vertices);
		IB = std::make_unique<DX12DefaultBuffer<UINT>>(device, commandList, indices);
	}
	else
	{
		UINT baseVertexLocation = 0;
		UINT baseIndexLocation = 0;
		UINT numVertices = 0;
		UINT numIndices = 0;

		baseVertexLocation = VB->GetNumDatas();
		baseIndexLocation = IB->GetNumDatas();
		numVertices = CGH::SizeTTransUINT(vertices.size());
		numIndices = CGH::SizeTTransUINT(indices.size());

		for (auto it : meshs)
		{
			for (auto& it2 : it.subs)
			{
				it2.second.vertexOffset += baseVertexLocation;
				it2.second.indexOffset += baseIndexLocation;
			}
		}

		result.VBResult = VB->AddData(device, commandList, numVertices, vertices.data());
		result.IBResult = IB->AddData(device, commandList, numIndices, indices.data());
	}

	for (size_t i = 0; i < meshNames.size(); i++)
	{
		MS.insert({ meshNames[i], meshs[i] });
	}

	return result;
}

template<typename Vertex_T>
inline bool DX12MeshSet<Vertex_T>::EditMesh(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, const std::string& meshName, const std::vector<Vertex>& vertices)
{
	auto iter = MS.find(meshName);

	if (iter == MS.end())
	{
		return false;
	}

	return 	VB->EditDatas(device, commandList, iter->second.GetStartVertexOffset(), CGH::SizeTTransUINT(vertices.size()), vertices.data());
}

template<typename Vertex_T>
inline void DX12MeshSet<Vertex_T>::ClearUploadBuffer()
{
	VB->ClearUploadBuffer();
	IB->ClearUploadBuffer();
}

template<typename Vertex_T>
inline D3D12_VERTEX_BUFFER_VIEW DX12MeshSet<Vertex_T>::GetVertexBufferView() const
{
	D3D12_VERTEX_BUFFER_VIEW result = {};

	result.BufferLocation = VB->GetBufferResource()->GetGPUVirtualAddress();
	result.SizeInBytes = CGH::SizeTTransUINT(VB->GetBufferSize());
	result.StrideInBytes = sizeof(Vertex_T);

	return result;
}

template<typename Vertex_T>
inline D3D12_INDEX_BUFFER_VIEW DX12MeshSet<Vertex_T>::GetIndexBufferView() const
{
	D3D12_INDEX_BUFFER_VIEW result = {};
	result.Format = DXGI_FORMAT_R32_UINT;
	result.BufferLocation = IB->GetBufferResource()->GetGPUVirtualAddress();
	result.SizeInBytes = CGH::SizeTTransUINT(IB->GetBufferSize());

	return result;
}
