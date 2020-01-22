#include "DX12DrawSetNormalMesh.h"
#include "DX12TextureBuffer.h"

void DX12DrawSetNormalMesh::Init(ID3D12Device* device)
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
	heapDesc.NumDescriptors = m_NumFrame;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	heapDesc.NodeMask = 1;

	ThrowIfFailed(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_CommandUAVHeap.GetAddressOf())));
	auto heapHandle = m_CommandUAVHeap->GetCPUDescriptorHandleForHeapStart();
	auto uavSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	for (int i = 0; i < m_NumFrame; i++)
	{
		m_MeshObjectCB.push_back(std::make_unique<DX12UploadBuffer<DX12ObjectConstants>>(device, 100, true));
		m_Commands.push_back(std::make_unique<DX12CommandBuffer<DX12NormalMeshIndirectCommand>>(device, 100, heapHandle));
		heapHandle.ptr += uavSize;
	}

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
	m_PSOCon->AddShader("normal", DX12_SHADER_VERTEX, L"../Common/MainShaders/BaseShader.hlsl", macros, "VS");
	m_PSOCon->AddShader("normal", DX12_SHADER_PIXEL, L"../Common/MainShaders/BaseShader.hlsl", macros, "PS");
	m_PSOCon->AddCommandSignature("normal", "normal", commandSignatureDesc);

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
	SetPSO(cmd, custom);
	SetBaseRoots(cmd);
	
	auto currIndiectCommand = m_Commands[m_CurrFrame]->Resource();
	auto objectCBVritualAD = m_MeshObjectCB[m_CurrFrame]->Resource()->GetGPUVirtualAddress();
	const UINT objectStrideSize = m_MeshObjectCB[m_CurrFrame]->GetElementByteSize();

	cmd->IASetVertexBuffers(0, 1, &m_MeshSet.GetVertexBufferView());
	cmd->IASetIndexBuffer(&m_MeshSet.GetIndexBufferView());
	cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	DX12NormalMeshIndirectCommand idc;
	for (size_t i = 0; i < m_RenderObjectSubmesh.size(); i++)
	{
		idc.cbv = objectCBVritualAD;
		idc.draw.IndexCountPerInstance = m_RenderObjectSubmesh[i]->numIndex;
		idc.draw.StartIndexLocation = m_RenderObjectSubmesh[i]->indexOffset;
		idc.draw.BaseVertexLocation = m_RenderObjectSubmesh[i]->vertexOffset;
		idc.draw.StartInstanceLocation = 0;
		idc.draw.InstanceCount = 1;
		
		objectCBVritualAD += objectStrideSize;
		m_ReservedCommands.push_back(idc);
	}

	DX12NormalMeshIndirectCommand* bufferPtr = nullptr;
	currIndiectCommand->Map(0, nullptr, reinterpret_cast<void**>(&bufferPtr));
	memcpy(bufferPtr, m_ReservedCommands.data(), m_ReservedCommands.size() * sizeof(DX12NormalMeshIndirectCommand));
	currIndiectCommand->Unmap(0, nullptr);

	cmd->ExecuteIndirect(m_PSOCon->GetCommandSignature("normal"), m_RenderObjectSubmesh.size(), currIndiectCommand, 0, currIndiectCommand,
		m_Commands[m_CurrFrame]->GetCommandBufferCounterOffset());
}

void DX12DrawSetNormalMesh::ReserveRender(const RenderInfo& info)
{
	auto& mesh = m_MeshSet.MS.find(info.meshOrTextureName)->second;

	DX12ObjectConstants data;
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

void DX12DrawSetNormalMesh::UpdateFrameCountAndClearWork()
{
	DX12DrawSet::UpdateFrameCountAndClearWork();
	m_RenderObjectSubmesh.clear();
	m_ReservedCommands.clear();
}

void DX12DrawSetNormalMesh::ResizeCurrFrameCB()
{

}
