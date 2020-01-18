#include "DX12DrawSetSkinnedMesh.h"
#include "DX12TextureBuffer.h"

void DX12DrawSetSkinnedMesh::Init(ID3D12Device* device)
{
	for (int i = 0; i < m_NumFrame; i++)
	{
		m_MeshObjectCB.push_back(std::make_unique<DX12UploadBuffer<ObjectConstants>>(device, 100, true));
		m_AniBoneCB.push_back(std::make_unique<DX12UploadBuffer<AniBoneMat>>(device, 100, true));
	}

	CD3DX12_ROOT_PARAMETER baseRootParam[ROOT_COUNT];
	BaseRootParamSetting(baseRootParam);
	baseRootParam[OBJECT_CB].InitAsConstantBufferView(1);
	baseRootParam[ANIBONE_CB].InitAsConstantBufferView(2);

	CD3DX12_ROOT_SIGNATURE_DESC rootDesc;
	rootDesc.Init(ROOT_COUNT, baseRootParam, _countof(m_StaticSamplers),
		m_StaticSamplers, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	std::string boneMaxMatrixNum = std::to_string(BONEMAXMATRIX);
	std::string textureNum = std::to_string(m_TextureBuffer->GetTexturesNum());
	D3D_SHADER_MACRO macros[] = {
	"MAXTEXTURE", textureNum.c_str(),
	"BONEMAXMATRIX", boneMaxMatrixNum.c_str(),
	"SKINNED_VERTEX_SAHDER",NULL,
	NULL, NULL };

	m_PSOA.primitive = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	m_PSOA.rootSig = "skin";
	m_PSOA.vs = "skin";
	m_PSOA.ps = "skin";
	m_PSOCon->AddRootSignature("skin", rootDesc);
	m_PSOCon->AddShader("skin", DX12_SHADER_VERTEX, L"../Common/MainShaders/BaseShader.hlsl", macros, "VS");
	m_PSOCon->AddShader("skin", DX12_SHADER_PIXEL, L"../Common/MainShaders/BaseShader.hlsl", macros, "PS");

	m_PSOA.input = "skin";
	m_PSOCon->AddInputLayout("skin",
		{
			{ "POSITION" ,0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "WEIGHTS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "BONEINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT, 0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		});
}

void DX12DrawSetSkinnedMesh::Draw(ID3D12GraphicsCommandList* cmd, const PSOAttributeNames* custom)
{
	SetPSO(cmd, custom);

	SetBaseRoots(cmd);

	auto ObjectCBVritualAD = m_MeshObjectCB[m_CurrFrame]->Resource()->GetGPUVirtualAddress();
	const UINT ObjectStrideSize = m_MeshObjectCB[m_CurrFrame]->GetElementByteSize();

	auto AniBoneCBVritualAD = m_AniBoneCB[m_CurrFrame]->Resource()->GetGPUVirtualAddress();
	const UINT AniBoneStrideSize = m_AniBoneCB[m_CurrFrame]->GetElementByteSize();

	cmd->IASetVertexBuffers(0, 1, &m_MeshSet.GetVertexBufferView());
	cmd->IASetIndexBuffer(&m_MeshSet.GetIndexBufferView());
	cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (size_t i = 0; i < m_RenderObjectSubmesh.size(); i++)
	{
		cmd->SetGraphicsRootConstantBufferView(OBJECT_CB, ObjectCBVritualAD);

		if (m_AniBoneIndices[i] > -1)
		{
			cmd->SetGraphicsRootConstantBufferView(ANIBONE_CB, 
				AniBoneCBVritualAD+(m_AniBoneIndices[i]* AniBoneStrideSize));
		}

		cmd->DrawIndexedInstanced(m_RenderObjectSubmesh[i]->numIndex, 1,
			m_RenderObjectSubmesh[i]->indexOffset, m_RenderObjectSubmesh[i]->vertexOffset, 0);
		ObjectCBVritualAD += ObjectStrideSize;
	}

	m_RenderObjectSubmesh.clear();
	m_AniBoneIndices.clear();
}

void DX12DrawSetSkinnedMesh::ReserveRender(const RenderInfo& info)
{
	auto& mesh = m_MeshSet.MS.find(info.meshOrTextureName)->second;
	
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
		m_AniBoneIndices.push_back(info.skin.aniBoneIndex);
	}
}

void DX12DrawSetSkinnedMesh::UpdateAniBoneCB(const std::vector<AniBoneMat>& reservedData)
{
	size_t index = 0;

	for (auto& it : reservedData)
	{
		m_AniBoneCB[m_CurrFrame]->CopyData(index, reservedData[index]);
		index++;
	}
}