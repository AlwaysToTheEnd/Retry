#include "DX12DrawSetLight.h"
#include "DX12TextureBuffer.h"

void DX12DrawSetLight::Init(ID3D12Device* device)
{
	const char* signatureName = "light";

	for (int i = 0; i < m_NumFrame; i++)
	{
		m_LightInfomations.push_back(std::make_unique<DX12UploadBuffer<DX12LightInfomation>>(device, 100, true));
	}

	CD3DX12_ROOT_PARAMETER baseRootParam[ROOT_COUNT];
	BaseRootParamSetting(baseRootParam);
	baseRootParam[LIGHTTYPE_CONST].InitAsConstants(1,1);

	CD3DX12_ROOT_SIGNATURE_DESC rootDesc;
	rootDesc.Init(ROOT_COUNT, baseRootParam, _countof(m_StaticSamplers),
		m_StaticSamplers, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	std::string textureNum = std::to_string(m_TextureBuffer->GetTexturesNum());
	D3D_SHADER_MACRO macros[] = {
		"MAXTEXTURE", textureNum.c_str(),
		NULL, NULL };

	D3D12_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = false;
	dsDesc.StencilEnable = false;

	m_PSOA.primitive = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
	m_PSOA.rootSig = signatureName;
	m_PSOA.depthStencil = signatureName;
	m_PSOA.vs = signatureName;
	m_PSOA.hs = signatureName;
	m_PSOA.ds = signatureName;
	m_PSOA.ps = signatureName;
	m_PSOCon->AddRootSignature(signatureName, rootDesc);
	m_PSOCon->AddDepthStencil(signatureName, dsDesc);
	m_PSOCon->AddShader(signatureName, DX12_SHADER_VERTEX, L"../Common/MainShaders/LightShader.hlsl", macros, "VS");
	m_PSOCon->AddShader(signatureName, DX12_SHADER_HULL, L"../Common/MainShaders/LightShader.hlsl", macros, "HS");
	m_PSOCon->AddShader(signatureName, DX12_SHADER_DOMAIN, L"../Common/MainShaders/LightShader.hlsl", macros, "DS");
	m_PSOCon->AddShader(signatureName, DX12_SHADER_PIXEL, L"../Common/MainShaders/LightShader.hlsl", macros, "PS");
	
	m_PSOA.input = signatureName;
	m_PSOCon->AddInputLayout(signatureName,
		{
			{ "POSITIONANGLE" ,0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,0,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
			{ "LIGHTCOLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "FALLOFFnPOWER", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "LIGHTDIR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 40, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		});
}

void DX12DrawSetLight::Draw(ID3D12GraphicsCommandList* cmd, const DX12PSOAttributeNames* custom, const DX12_COMPUTE_CULLING_DESC* culling)
{
	if (m_RenderCount)
	{
		////////////////////////////////////////////////////////////////////////////////////
		SetPSO(cmd, custom);
		SetBaseRoots(cmd);

		D3D12_VERTEX_BUFFER_VIEW VB = {};

		VB.BufferLocation = m_LightInfomations[m_CurrFrame]->Resource()->GetGPUVirtualAddress();
		VB.SizeInBytes = m_LightInfomations[m_CurrFrame]->GetBufferSize();
		VB.StrideInBytes = sizeof(DX12LightInfomation);

		cmd->IASetVertexBuffers(0, 1, &VB);
		cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);
		cmd->SetGraphicsRoot32BitConstant(LIGHTTYPE_CONST, 0, 0);
		cmd->DrawInstanced(m_RenderCount, 1, 0, 0);
	}
}

void DX12DrawSetLight::ReserveRender(const RenderInfo& info)
{
	DX12LightInfomation data;
	data.falloffAndPower = info.lightInfo.falloffAndPower;
	data.lightColor = info.lightInfo.color;
	data.posnAngle = physx::PxVec4(info.world.getPosition(), info.lightInfo.angle);
	data.dir = info.world.rotate(physx::PxVec3(0, 0, 1));

	m_LightInfomations[m_CurrFrame]->CopyData(m_RenderCount, data);
	m_RenderCount++;
}

void DX12DrawSetLight::UpdateFrameCountAndClearWork()
{
	DX12DrawSet::UpdateFrameCountAndClearWork();
	m_RenderCount = 0;
}
