#include "DX12DrawSetHeightField.h"
#include "DX12TextureBuffer.h"

void DX12DrawSetHeightField::Init(ID3D12Device* device)
{
	for (int i = 0; i < m_NumFrame; i++)
	{
		m_MeshObjectCB.push_back(std::make_unique<DX12UploadBuffer<ObjectConstants>>(device, 100, true));
	}

	m_FieldInfo = std::make_unique<DX12UploadBuffer<FieldInfo>>(device, 1, true);

	m_PSOA.primitive = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	m_PSOA.rootSig = "normal";
	m_PSOA.vs = "normal";
	m_PSOA.ps = "normal";

	m_PSOA.input = "normal";

	CD3DX12_ROOT_PARAMETER computeRootParam[COMPUTE_ROOT_COUNT];
	computeRootParam[COMPUTE_HEIGHTS_SRV].InitAsShaderResourceView(0);
	computeRootParam[COMPUTE_VERTICES_UAV].InitAsUnorderedAccessView(0);
	computeRootParam[COMPUTE_INDICES_UAV].InitAsUnorderedAccessView(1);
	computeRootParam[COMPUTE_FIELDINFO_CB].InitAsConstantBufferView(0);

	CD3DX12_ROOT_SIGNATURE_DESC computeRoot;
	computeRoot.Init(COMPUTE_ROOT_COUNT, computeRootParam);

	m_PSOCon->AddRootSignature(heightFieldCreateCSName, computeRoot);
	m_PSOCon->AddShader(heightFieldCreateCSName+"0", DX12_SHADER_COMPUTE, L"../Common/MainShaders/HeightFieldMeshCreate.hlsl", nullptr, "SettingVerticesPos");
	m_PSOCon->AddShader(heightFieldCreateCSName+"1", DX12_SHADER_COMPUTE, L"../Common/MainShaders/HeightFieldMeshCreate.hlsl", nullptr, "SettingVerticesNormalAndIndices");
}

void DX12DrawSetHeightField::Draw(ID3D12GraphicsCommandList* cmd, const PSOAttributeNames* custom)
{
	if (m_MeshSet.VB.get())
	{
		SetPSO(cmd, custom);
		SetBaseRoots(cmd);

		auto ObjectCBVritualAD = m_MeshObjectCB[m_CurrFrame]->Resource()->GetGPUVirtualAddress();
		const UINT ObjectStrideSize = m_MeshObjectCB[m_CurrFrame]->GetElementByteSize();

		cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		for (size_t i = 0; i < m_TargetMeshs.size(); i++)
		{
			D3D12_VERTEX_BUFFER_VIEW vertexView = {};
			vertexView.BufferLocation = m_TargetMeshs[i]->resultVertices->GetGPUVirtualAddress();
			vertexView.SizeInBytes = m_TargetMeshs[i]->numVertices* sizeof(Vertex);
			vertexView.StrideInBytes = sizeof(Vertex);

			D3D12_INDEX_BUFFER_VIEW indexView = {};
			indexView.Format = DXGI_FORMAT_R32_UINT;
			indexView.BufferLocation = m_TargetMeshs[i]->resultIndices->GetGPUVirtualAddress();
			indexView.SizeInBytes = m_TargetMeshs[i]->numIndices * sizeof(UINT);

			cmd->IASetVertexBuffers(0, 1, &vertexView);
			cmd->IASetIndexBuffer(&indexView);

			cmd->SetGraphicsRootConstantBufferView(OBJECT_CB, ObjectCBVritualAD);
			cmd->DrawIndexedInstanced(m_TargetMeshs[i]->numIndices, 1,
				0, 0, 0);
			ObjectCBVritualAD += ObjectStrideSize;
		}

	}
}

void DX12DrawSetHeightField::ReserveRender(const RenderInfo& info)
{
	auto& mesh = m_MeshSet.MS.find(info.meshOrTextureName)->second;

	ObjectConstants data;
	data.world = info.world;

	assert(mesh.IsOneSub());
	for (auto& it : mesh.subs)
	{
		data.materialIndex = m_MaterialBuffer->GetIndex(it.second.material);
		data.diffuseMapIndex = m_TextureBuffer->GetTextureIndex(it.second.diffuseMap);
		data.normalMapIndex = m_TextureBuffer->GetTextureIndex(it.second.normalMap);

		m_MeshObjectCB[m_CurrFrame]->CopyData(m_TargetMeshs.size(), data);
	}

	m_TargetMeshs.push_back(&m_ResultMesh.find(info.meshOrTextureName)->second);
}

void DX12DrawSetHeightField::UpdateFrameCountAndClearWork()
{
	DX12DrawSet::UpdateFrameCountAndClearWork();
	m_TargetMeshs.clear();
}

void DX12DrawSetHeightField::ReComputeHeightField(const std::string& name, physx::PxVec3 scale, ID3D12Device* device, ID3D12GraphicsCommandList* cmd)
{
	auto meshIter = m_MeshSet.MS.find(name);
	assert(meshIter != m_MeshSet.MS.end());

	auto resultMesh = m_ResultMesh.find(name);

	if (resultMesh==m_ResultMesh.end())
	{
		auto& newMesh = m_ResultMesh[name];

		newMesh.numVertices = meshIter->second.GetTotalVertexNum();
		newMesh.mapSize = sqrt(newMesh.numVertices);
		newMesh.numIndices = 6 * (newMesh.mapSize) * (newMesh.mapSize);

		D3D12_HEAP_PROPERTIES vhp = {};
		vhp.Type = D3D12_HEAP_TYPE_DEFAULT;
		vhp.CreationNodeMask = 1;
		vhp.VisibleNodeMask = 1;

		D3D12_RESOURCE_DESC vhrd = {};
		vhrd.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		vhrd.Width = newMesh.numVertices * sizeof(Vertex);
		vhrd.Height = 1;
		vhrd.DepthOrArraySize = 1;
		vhrd.MipLevels = 1;
		vhrd.Format = DXGI_FORMAT_UNKNOWN;
		vhrd.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		vhrd.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		vhrd.SampleDesc.Count = 1;
		vhrd.SampleDesc.Quality = 0;

		ThrowIfFailed(device->CreateCommittedResource(&vhp, D3D12_HEAP_FLAG_NONE, &vhrd, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			nullptr, IID_PPV_ARGS(newMesh.resultVertices.GetAddressOf())));

		vhrd.Width = newMesh.numIndices * sizeof(UINT);

		ThrowIfFailed(device->CreateCommittedResource(&vhp, D3D12_HEAP_FLAG_NONE, &vhrd, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			nullptr, IID_PPV_ARGS(newMesh.resultIndices.GetAddressOf())));

		resultMesh = m_ResultMesh.find(name);
	}
	else
	{
		D3D12_RESOURCE_BARRIER barrier[2] = {};
		barrier[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier[0].Transition.pResource = resultMesh->second.resultVertices.Get();
		barrier[0].Transition.StateBefore = D3D12_RESOURCE_STATE_GENERIC_READ;
		barrier[0].Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

		barrier[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier[1].Transition.pResource = resultMesh->second.resultIndices.Get();
		barrier[1].Transition.StateBefore = D3D12_RESOURCE_STATE_GENERIC_READ;
		barrier[1].Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

		cmd->ResourceBarrier(_countof(barrier), barrier);
	}

	FieldInfo info;
	info.Scale = scale;
	info.MapSize = resultMesh->second.mapSize;
	info.NumVertices = resultMesh->second.numVertices;
	m_FieldInfo->CopyData(0, info);

	m_PSOCon->SetPSOToCommnadList(cmd, heightFieldCreateCSName, heightFieldCreateCSName + "0");

	cmd->SetComputeRootShaderResourceView(COMPUTE_HEIGHTS_SRV,
		m_MeshSet.VB->GetBufferResource()->GetGPUVirtualAddress() + (meshIter->second.GetStartVertexOffset() * sizeof(float)));

	cmd->SetComputeRootUnorderedAccessView(COMPUTE_VERTICES_UAV, resultMesh->second.resultVertices->GetGPUVirtualAddress());
	cmd->SetComputeRootUnorderedAccessView(COMPUTE_INDICES_UAV, resultMesh->second.resultIndices->GetGPUVirtualAddress());
	cmd->SetComputeRootConstantBufferView(COMPUTE_FIELDINFO_CB, m_FieldInfo->Resource()->GetGPUVirtualAddress());

	cmd->Dispatch(info.NumVertices, 1, 1);

	m_PSOCon->SetPSOToCommnadList(cmd, heightFieldCreateCSName, heightFieldCreateCSName + "1");

	cmd->Dispatch(info.NumVertices, 1, 1);

	D3D12_RESOURCE_BARRIER barrier[2] = {};
	barrier[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier[0].Transition.pResource = resultMesh->second.resultVertices.Get();
	barrier[0].Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	barrier[0].Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;

	barrier[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier[1].Transition.pResource = resultMesh->second.resultIndices.Get();
	barrier[1].Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	barrier[1].Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;

	cmd->ResourceBarrier(_countof(barrier), barrier);
}
