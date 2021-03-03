#include "DX12DrawSetSkinnedMesh.h"
#include "DX12TextureBuffer.h"

void DX12DrawSetSkinnedMesh::Init(ID3D12Device* device)
{
	FrameUploadSRVs srvs;
	for (unsigned int i = 0; i < m_NumFrame; i++)
	{
		m_MeshObjectCB.push_back(std::make_unique<DX12UploadBuffer<DX12ObjectConstants>>(device, 100, true));
		m_AniBoneCB.push_back(std::make_unique<DX12UploadBuffer<AniBoneMat>>(device, 100, true));
		m_ReservedCommands.push_back(std::make_unique<DX12UploadBuffer<DX12SkinnedMeshIndirectCommand>>(device, 100, false));
		srvs.push_back(m_ReservedCommands.back()->Resource());
	}

	m_Culling.Init(device, m_PSOCon, m_MeshObjectCB, srvs, 100, sizeof(DX12SkinnedMeshIndirectCommand));

	CD3DX12_ROOT_PARAMETER baseRootParam[ROOT_COUNT];
	BaseRootParamSetting(baseRootParam);
	baseRootParam[OBJECT_CB].InitAsConstantBufferView(1);
	baseRootParam[ANIBONE_CB].InitAsConstantBufferView(2);

	CD3DX12_ROOT_SIGNATURE_DESC rootDesc;
	rootDesc.Init(ROOT_COUNT, baseRootParam, _countof(m_StaticSamplers),
		m_StaticSamplers, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	D3D12_INDIRECT_ARGUMENT_DESC argumentDescs[3] = {};
	argumentDescs[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT_BUFFER_VIEW;
	argumentDescs[0].ConstantBufferView.RootParameterIndex = OBJECT_CB;
	argumentDescs[1].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT_BUFFER_VIEW;
	argumentDescs[1].ConstantBufferView.RootParameterIndex = ANIBONE_CB;
	argumentDescs[2].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;

	D3D12_COMMAND_SIGNATURE_DESC commandSignatureDesc = {};
	commandSignatureDesc.pArgumentDescs = argumentDescs;
	commandSignatureDesc.NumArgumentDescs = _countof(argumentDescs);
	commandSignatureDesc.ByteStride = sizeof(DX12SkinnedMeshIndirectCommand);

	std::string boneMaxMatrixNum = std::to_string(BONEMAXMATRIX);
	std::string textureNum = std::to_string(m_TextureBuffer->GetTexturesNum());
	D3D_SHADER_MACRO macros[] = {
	"MAXTEXTURE", textureNum.c_str(),
	"BONEMAXMATRIX", boneMaxMatrixNum.c_str(),
	"SKINNED_VERTEX_SAHDER",NULL,
	NULL, NULL, NULL, NULL };

	m_PSOA.primitive = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	m_PSOA.rootSig = "skin";
	m_PSOA.vs = "skin";
	m_PSOA.ps = "skin";
	m_PSOCon->AddRootSignature("skin", rootDesc);
	m_PSOCon->AddCommandSignature("skin", "skin", commandSignatureDesc);
	m_PSOCon->AddShader("skin", DX12_SHADER_VERTEX, L"../Common/MainShaders/MeshShader.hlsl", macros, "VS");
	m_PSOCon->AddShader("skin", DX12_SHADER_PIXEL, L"../Common/MainShaders/MeshShader.hlsl", macros, "PS");
	
	m_PSOA.input = "skin";
	m_PSOCon->AddInputLayout("skin",
		{
			{ "POSITION" ,0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "WEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "WEIGHTS", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 48, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "BONEINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT, 0, 64, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "BONEINDICES", 1, DXGI_FORMAT_R8G8B8A8_UINT, 0, 68, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		});
}

void DX12DrawSetSkinnedMesh::Draw(ID3D12GraphicsCommandList* cmd, const DX12PSOAttributeNames* custom, const DX12_COMPUTE_CULLING_DESC* culling)
{
	if (m_RenderCount)
	{
		auto result = m_Culling.RenderCompute(cmd, m_RenderCount, m_CurrFrame, "skinnedCulling", culling);
		////////////////////////////////////////////////////////////////////////////////////

		SetPSO(cmd, custom);
		SetBaseRoots(cmd);

		cmd->IASetVertexBuffers(0, 1, &m_MeshSet.GetVertexBufferView());
		cmd->IASetIndexBuffer(&m_MeshSet.GetIndexBufferView());
		cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		cmd->ExecuteIndirect(m_PSOCon->GetCommandSignature("skin"), m_RenderCount,
			result, 0, result, m_Culling.GetCounterOffset());
	}
}

void DX12DrawSetSkinnedMesh::ReserveRender(const RenderInfo& info)
{
	auto& mesh = m_MeshSet.MS.find(info.meshOrTextureName)->second;

	auto objectCBVritualAD = m_MeshObjectCB[m_CurrFrame]->Resource()->GetGPUVirtualAddress();
	const UINT objectStrideSize = m_MeshObjectCB[m_CurrFrame]->GetElementByteSize();

	auto AniBoneCBVritualAD = m_AniBoneCB[m_CurrFrame]->Resource()->GetGPUVirtualAddress();
	const UINT AniBoneStrideSize = m_AniBoneCB[m_CurrFrame]->GetElementByteSize();

	DX12SkinnedMeshIndirectCommand idc;
	DX12ObjectConstants data;
	data.world = info.world;
	data.scale = info.scale;
	data.boundSphereRad = info.cullingBoundSphereRad;
	data.objectID = info.pixelColID;

	for (auto& it : mesh.subs)
	{
		data.materialIndex = m_MaterialBuffer->GetIndex(it.second.material);
		data.diffuseMapIndex = m_TextureBuffer->GetTextureIndex(it.second.diffuseMap);
		data.normalMapIndex = m_TextureBuffer->GetTextureIndex(it.second.normalMap);

		m_MeshObjectCB[m_CurrFrame]->CopyData(m_RenderCount, data);

		if (info.skin.aniBoneIndex > -1)
		{
			idc.aniBoneCbv = AniBoneCBVritualAD + (static_cast<D3D12_GPU_VIRTUAL_ADDRESS>(info.skin.aniBoneIndex) * AniBoneStrideSize);
		}
		else
		{
			idc.aniBoneCbv = AniBoneCBVritualAD;
		}

		idc.objectCbv = objectCBVritualAD + (static_cast<D3D12_GPU_VIRTUAL_ADDRESS>(m_RenderCount) * objectStrideSize);
		idc.draw.IndexCountPerInstance = it.second.numIndex;
		idc.draw.StartIndexLocation = it.second.indexOffset;
		idc.draw.BaseVertexLocation = it.second.vertexOffset;
		idc.draw.InstanceCount = 1;
		idc.draw.StartInstanceLocation = 0;

		m_ReservedCommands[m_CurrFrame]->CopyData(m_RenderCount, idc);

		m_RenderCount++;
	}
}

void DX12DrawSetSkinnedMesh::UpdateFrameCountAndClearWork()
{
	DX12DrawSet::UpdateFrameCountAndClearWork();
	m_RenderCount = 0;
}

void DX12DrawSetSkinnedMesh::UpdateAniBoneCB(const std::vector<AniBoneMat>& reservedData)
{
	int index = 0;

	for (auto& it : reservedData)
	{
		m_AniBoneCB[m_CurrFrame]->CopyData(index, reservedData[index]);
		index++;
	}
}

std::string DX12DrawSetSkinnedMesh::GetShadowRenderShaderName(DX12_SHADER_TYPE type)
{

	return std::string();
}
