#include "DX12DrawSetNormalMesh.h"
#include "DX12TextureBuffer.h"

void DX12DrawSetNormalMesh::Init(ID3D12Device* device, PSOController* psoCon,
	DXGI_FORMAT rtvFormat, DXGI_FORMAT dsvFormat,
	DX12TextureBuffer* textureBuffer,
	DX12IndexManagementBuffer<Material>* material, ID3D12Resource* mainPass)
{
	for (int i = 0; i < m_NumFrame; i++)
	{
		m_MeshObjectCB.push_back(std::make_unique<DX12UploadBuffer<ObjectConstants>>(device, 100, true));
	}

	m_PSOA.rtvFormats.push_back(rtvFormat);
	m_PSOA.dsvFormat = dsvFormat;
	m_TextureBuffer = textureBuffer;
	m_MaterialBuffer = material;
	m_MainPassCB = mainPass;
	m_PSOCon = psoCon;

	CD3DX12_DESCRIPTOR_RANGE texTable;
	texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, m_TextureBuffer->GetTexturesNum(), 0);

	CD3DX12_ROOT_PARAMETER baseRootParam[ROOT_COUNT];
	baseRootParam[PASS_CB].InitAsConstantBufferView(0);
	baseRootParam[TEXTURE_TABLE].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);
	baseRootParam[MATERIAL_SRV].InitAsShaderResourceView(0, 1);
	baseRootParam[OBJECT_CB].InitAsConstantBufferView(1);

	CD3DX12_ROOT_SIGNATURE_DESC rootDesc;
	rootDesc.Init(ROOT_COUNT, baseRootParam, _countof(m_StaticSamplers),
		m_StaticSamplers, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	std::string textureNum = std::to_string(m_TextureBuffer->GetTexturesNum());
	D3D_SHADER_MACRO macros[] = {
		"MAXTEXTURE", textureNum.c_str(),
		NULL, NULL };

	m_PSOA.primitive = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	m_PSOA.rootSig = "normal";
	m_PSOA.vs = "normal";
	m_PSOA.ps = "normal";
	m_PSOCon->AddRootSignature("normal", rootDesc);
	m_PSOCon->AddShader("normal", DX12_SHADER_VERTEX, L"../Common/MainShaders/BaseShader.hlsl", macros, "VS");
	m_PSOCon->AddShader("normal", DX12_SHADER_PIXEL, L"../Common/MainShaders/BaseShader.hlsl", macros, "PS");

	m_PSOA.input = "normal";
	m_PSOCon->AddInputLayout("normal",
		{
			{ "POSITION" ,0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		});
}

void DX12DrawSetNormalMesh::Draw(ID3D12GraphicsCommandList* cmd, const PSOAttributeNames* custom)
{
	SetPSO(cmd, custom);

	ID3D12DescriptorHeap* descriptorHeaps[] = { m_TextureBuffer->GetHeap() };
	cmd->SetDescriptorHeaps(1, descriptorHeaps);

	cmd->SetGraphicsRootShaderResourceView(MATERIAL_SRV, m_MaterialBuffer->GetBufferResource()->GetGPUVirtualAddress());
	cmd->SetGraphicsRootConstantBufferView(PASS_CB, GetCurrMainPassAddress());
	cmd->SetGraphicsRootDescriptorTable(TEXTURE_TABLE, m_TextureBuffer->GetHeap()->GetGPUDescriptorHandleForHeapStart());

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
	D3D12_INDEX_BUFFER_VIEW indexBufferView = {};
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;

	auto ObjectCBVritualAD = m_MeshObjectCB[m_CurrFrame]->Resource()->GetGPUVirtualAddress();
	const UINT ObjectStrideSize = m_MeshObjectCB[m_CurrFrame]->GetElementByteSize();

	vertexBufferView.BufferLocation = m_VertexBuffer->GetBufferResource()->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = m_VertexBuffer->GetBufferSize();
	vertexBufferView.StrideInBytes = sizeof(Vertex);

	indexBufferView.BufferLocation = m_IndexBuffer->GetBufferResource()->GetGPUVirtualAddress();
	indexBufferView.SizeInBytes = m_IndexBuffer->GetBufferSize();

	cmd->IASetVertexBuffers(0, 1, &vertexBufferView);
	cmd->IASetIndexBuffer(&indexBufferView);
	cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (size_t i = 0; i < m_RenderObjectSubmesh.size(); i++)
	{
		cmd->SetGraphicsRootConstantBufferView(OBJECT_CB, ObjectCBVritualAD);
		cmd->DrawIndexedInstanced(m_RenderObjectSubmesh[i]->numIndex, 1,
			m_RenderObjectSubmesh[i]->indexOffset, m_RenderObjectSubmesh[i]->vertexOffset, 0);
		ObjectCBVritualAD += ObjectStrideSize;
	}

	m_RenderObjectSubmesh.clear();
}

void DX12DrawSetNormalMesh::ReserveRender(const RenderInfo& info)
{
	auto& mesh = m_Meshs.find(info.meshOrTextureName)->second;

	ObjectConstants data;
	data.world = info.world;
	data.scale = info.scale;

	for (auto& it : mesh.subs)
	{
		data.materialIndex = m_MaterialBuffer->GetIndex(it.second.material);
		data.diffuseMapIndex = m_TextureBuffer->GetTextureIndex(it.second.diffuseMap);
		data.normalMapIndex = m_TextureBuffer->GetTextureIndex(it.second.normalMap);

		m_MeshObjectCB[m_CurrFrame]->CopyData(m_RenderObjectSubmesh.size(), data);
		m_RenderObjectSubmesh.push_back(&it.second);
	}
}

bool DX12DrawSetNormalMesh::AddMesh(ID3D12Device* device, ID3D12GraphicsCommandList* commandList,
	const std::string& name, MeshObject& mesh,
	const std::vector<Vertex>& vertices, const std::vector<UINT>& indices)
{
	if (m_VertexBuffer == nullptr)
	{
		CreateVertexIndexBuffer(device, commandList, vertices, indices);
	}
	else
	{
		UINT baseVertexLocation = 0;
		UINT baseIndexLocation = 0;
		UINT numVertices = 0;
		UINT numIndices = 0;

		baseVertexLocation = m_VertexBuffer->GetNumDatas();
		baseIndexLocation = m_IndexBuffer->GetNumDatas();
		numVertices = vertices.size();
		numIndices = indices.size();

		for (auto& it : mesh.subs)
		{
			it.second.vertexOffset += baseVertexLocation;
			it.second.indexOffset += baseIndexLocation;
		}

		m_VertexBuffer->AddData(device, commandList, numVertices, vertices.data());
		m_IndexBuffer->AddData(device, commandList, numIndices, indices.data());
	}

	m_Meshs.insert({ name, mesh });

	return true;
}

bool DX12DrawSetNormalMesh::AddMeshs(ID3D12Device* device, ID3D12GraphicsCommandList* commandList,
	const std::vector<std::string>& meshNames, const std::vector<MeshObject>& meshs,
	const std::vector<Vertex>& vertices, const std::vector<UINT>& indices)
{
	if (m_VertexBuffer == nullptr)
	{
		CreateVertexIndexBuffer(device, commandList, vertices, indices);
	}
	else
	{
		UINT baseVertexLocation = 0;
		UINT baseIndexLocation = 0;
		UINT numVertices = 0;
		UINT numIndices = 0;

		baseVertexLocation = m_VertexBuffer->GetNumDatas();
		baseIndexLocation = m_IndexBuffer->GetNumDatas();
		numVertices = vertices.size();
		numIndices = indices.size();

		for (auto it : meshs)
		{
			for (auto& it2 : it.subs)
			{
				it2.second.vertexOffset += baseVertexLocation;
				it2.second.indexOffset += baseIndexLocation;
			}
		}
		
		m_VertexBuffer->AddData(device, commandList, numVertices, vertices.data());
		m_IndexBuffer->AddData(device, commandList, numIndices, indices.data());
	}

	for (size_t i = 0; i < meshNames.size(); i++)
	{
		m_Meshs.insert({ meshNames[i], meshs[i] });
	}

	return true;
}

bool DX12DrawSetNormalMesh::EditMesh(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, const std::string& meshName, const std::vector<Vertex>& vertices)
{
	auto iter = m_Meshs.find(meshName);

	if (iter == m_Meshs.end())
	{
		return false;
	}

	return 	m_VertexBuffer->EditDatas(device, commandList, iter->second.GetStartVertexOffset(), vertices.size(), vertices.data());
}

void DX12DrawSetNormalMesh::UploadBuffersClear()
{
	m_VertexBuffer->ClearUploadBuffer();
	m_IndexBuffer->ClearUploadBuffer();
}

void DX12DrawSetNormalMesh::CreateVertexIndexBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, const std::vector<Vertex>& vertices, const std::vector<UINT>& indices)
{
	m_VertexBuffer = std::make_unique<DX12DefaultBuffer<Vertex>>(device, commandList, vertices);
	m_IndexBuffer = std::make_unique<DX12DefaultBuffer<UINT>>(device, commandList, indices);
}

void DX12DrawSetNormalMesh::ResizeCurrFrameCB()
{

}
