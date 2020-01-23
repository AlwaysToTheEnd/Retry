#include "DX12DrawSetNormalMesh.h"
#include "DX12TextureBuffer.h"

void DX12DrawSetNormalMesh::Init(ID3D12Device* device)
{
	for (int i = 0; i < m_NumFrame; i++)
	{
		m_MeshObjectCB.push_back(std::make_unique<DX12UploadBuffer<DX12ObjectConstants>>(device, 100, true));
		m_ReservedCommands.push_back(std::make_unique<DX12UploadBuffer<DX12NormalMeshIndirectCommand>>(device, 100, false));
	}

	m_Culling.Init(device, m_PSOCon, m_MeshObjectCB, 100, sizeof(DX12NormalMeshIndirectCommand));

	CD3DX12_ROOT_PARAMETER baseRootParam[ROOT_COUNT];
	BaseRootParamSetting(baseRootParam);
	baseRootParam[OBJECT_CB].InitAsConstantBufferView(1);

	CD3DX12_ROOT_SIGNATURE_DESC rootDesc;
	rootDesc.Init(ROOT_COUNT, baseRootParam, _countof(m_StaticSamplers),
		m_StaticSamplers, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	std::string textureNum = std::to_string(m_TextureBuffer->GetTexturesNum());
	D3D_SHADER_MACRO macros[] = {
		"MAXTEXTURE", textureNum.c_str(),
		NULL, NULL };

	D3D12_INDIRECT_ARGUMENT_DESC argumentDescs[2] = {};
	argumentDescs[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT_BUFFER_VIEW;
	argumentDescs[0].ConstantBufferView.RootParameterIndex = OBJECT_CB;
	argumentDescs[1].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;

	D3D12_COMMAND_SIGNATURE_DESC commandSignatureDesc = {};
	commandSignatureDesc.pArgumentDescs = argumentDescs;
	commandSignatureDesc.NumArgumentDescs = _countof(argumentDescs);
	commandSignatureDesc.ByteStride = sizeof(DX12NormalMeshIndirectCommand);

	m_PSOA.primitive = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	m_PSOA.rootSig = "normal";
	m_PSOA.vs = "normal";
	m_PSOA.ps = "normal";
	m_PSOCon->AddRootSignature("normal", rootDesc);
	m_PSOCon->AddCommandSignature("normal", "normal", commandSignatureDesc);
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

void DX12DrawSetNormalMesh::Draw(ID3D12GraphicsCommandList* cmd, const DX12PSOAttributeNames* custom)
{
	auto objectCBVritualAD = m_MeshObjectCB[m_CurrFrame]->Resource()->GetGPUVirtualAddress();
	const UINT objectStrideSize = m_MeshObjectCB[m_CurrFrame]->GetElementByteSize();

	if (m_RenderCount)
	{
		auto result = m_Culling.Compute(cmd, m_RenderCount, m_ReservedCommands[m_CurrFrame]->Resource(), m_CurrFrame, "normalCulling");
		////////////////////////////////////////////////////////////////////////////////////

		SetPSO(cmd, custom);
		SetBaseRoots(cmd);

		cmd->IASetVertexBuffers(0, 1, &m_MeshSet.GetVertexBufferView());
		cmd->IASetIndexBuffer(&m_MeshSet.GetIndexBufferView());
		cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		cmd->ExecuteIndirect(m_PSOCon->GetCommandSignature("normal"), m_RenderCount,
			result, 0, result, m_Culling.GetCounterOffset());
	}
}

void DX12DrawSetNormalMesh::ReserveRender(const RenderInfo& info)
{
	auto& mesh = m_MeshSet.MS.find(info.meshOrTextureName)->second;
	auto objectCBVritualAD = m_MeshObjectCB[m_CurrFrame]->Resource()->GetGPUVirtualAddress();
	const UINT objectStrideSize = m_MeshObjectCB[m_CurrFrame]->GetElementByteSize();

	DX12NormalMeshIndirectCommand idc;
	DX12ObjectConstants data;
	data.world = info.world;
	data.scale = info.scale;

	for (auto& it : mesh.subs)
	{
		data.materialIndex = m_MaterialBuffer->GetIndex(it.second.material);
		data.diffuseMapIndex = m_TextureBuffer->GetTextureIndex(it.second.diffuseMap);
		data.normalMapIndex = m_TextureBuffer->GetTextureIndex(it.second.normalMap);

		m_MeshObjectCB[m_CurrFrame]->CopyData(m_RenderCount, data);

		idc.cbv = objectCBVritualAD + (m_RenderCount * objectStrideSize);
		idc.draw.IndexCountPerInstance = it.second.numIndex;
		idc.draw.StartIndexLocation = it.second.indexOffset;
		idc.draw.BaseVertexLocation = it.second.vertexOffset;
		idc.draw.InstanceCount = 1;
		idc.draw.StartInstanceLocation = 0;

		m_ReservedCommands[m_CurrFrame]->CopyData(m_RenderCount, idc);

		m_RenderCount++;
	}
}

void DX12DrawSetNormalMesh::UpdateFrameCountAndClearWork()
{
	DX12DrawSet::UpdateFrameCountAndClearWork();
	m_RenderCount = 0;
}

void DX12DrawSetNormalMesh::ResizeCurrFrameCB()
{

}
