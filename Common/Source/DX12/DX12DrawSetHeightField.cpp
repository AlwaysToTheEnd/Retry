#include "DX12DrawSetHeightField.h"
#include "DX12TextureBuffer.h"

void DX12DrawSetHeightField::Init(ID3D12Device* device)
{
	for (int i = 0; i < m_NumFrame; i++)
	{
		m_MeshObjectCB.push_back(std::make_unique<DX12UploadBuffer<HFObjectData>>(device, 10, true));
	}

	CD3DX12_ROOT_PARAMETER baseRootParam[ROOT_COUNT];
	BaseRootParamSetting(baseRootParam);
	baseRootParam[OBJECT_CB].InitAsConstantBufferView(1);
	baseRootParam[HEIGHT_SRV].InitAsShaderResourceView(1,1);

	CD3DX12_ROOT_SIGNATURE_DESC rootDesc;
	rootDesc.Init(ROOT_COUNT, baseRootParam, _countof(m_StaticSamplers),
		m_StaticSamplers, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	std::string textureNum = std::to_string(m_TextureBuffer->GetTexturesNum());
	D3D_SHADER_MACRO macros[] = {
		"MAXTEXTURE", textureNum.c_str(),
		NULL, NULL };

	m_PSOA.primitive = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	m_PSOA.rootSig = "HF";
	m_PSOA.vs = "HF";
	m_PSOA.gs = "HF";
	m_PSOA.ps = "HF";
	m_PSOCon->AddRootSignature("HF", rootDesc);
	m_PSOCon->AddShader("HF", DX12_SHADER_VERTEX, L"../Common/MainShaders/HeightFieldShader.hlsl", macros, "VS");
	m_PSOCon->AddShader("HF", DX12_SHADER_GEOMETRY, L"../Common/MainShaders/HeightFieldShader.hlsl", macros, "GS");
	m_PSOCon->AddShader("HF", DX12_SHADER_PIXEL, L"../Common/MainShaders/HeightFieldShader.hlsl", macros, "PS");

	m_PSOA.input = "HF";
	m_PSOCon->AddInputLayout("HF",
		{
			{ "HEIGHT" , 0, DXGI_FORMAT_R32_FLOAT, 0,0,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		});
}

void DX12DrawSetHeightField::Draw(ID3D12GraphicsCommandList* cmd, const PSOAttributeNames* custom)
{
	if (m_MeshSet.VB.get())
	{
		SetPSO(cmd, custom);
		SetBaseRoots(cmd);

		auto ObjectCBVritualAD = m_MeshObjectCB[m_CurrFrame]->Resource()->GetGPUVirtualAddress();
		const UINT ObjectStrideSize = m_MeshObjectCB[m_CurrFrame]->GetElementByteSize();

		auto HeightSrvVirtualAD = m_MeshSet.VB->GetBufferResource()->GetGPUVirtualAddress();

		cmd->IASetVertexBuffers(0, 1, &m_MeshSet.GetVertexBufferView());
		cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

		for (size_t i = 0; i < m_RenderObjectSubmesh.size(); i++)
		{
			cmd->SetGraphicsRootConstantBufferView(OBJECT_CB, ObjectCBVritualAD);
			cmd->SetGraphicsRootShaderResourceView(HEIGHT_SRV, HeightSrvVirtualAD+(m_RenderObjectSubmesh[i]->vertexOffset*sizeof(float)));
			cmd->DrawInstanced(m_RenderObjectSubmesh[i]->numVertex, 1,
				m_RenderObjectSubmesh[i]->vertexOffset, 0);

			ObjectCBVritualAD += ObjectStrideSize;
		}

		m_RenderObjectSubmesh.clear();
	}
}

void DX12DrawSetHeightField::ReserveRender(const RenderInfo& info)
{
	auto& mesh = m_MeshSet.MS.find(info.meshOrTextureName)->second;

	HFObjectData data;
	data.world = info.world;
	data.scale = info.scale;

	for (auto& it : mesh.subs)
	{
		data.materialIndex = m_MaterialBuffer->GetIndex(it.second.material);
		data.diffuseMapIndex = m_TextureBuffer->GetTextureIndex(it.second.diffuseMap);
		data.normalMapIndex = m_TextureBuffer->GetTextureIndex(it.second.normalMap);

		data.mapSize = sqrt(it.second.numVertex);
		data.numVertices = it.second.numVertex;
		m_MeshObjectCB[m_CurrFrame]->CopyData(m_RenderObjectSubmesh.size(), data);

		m_RenderObjectSubmesh.push_back(&it.second);
	}
}