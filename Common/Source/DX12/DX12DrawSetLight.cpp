#include "DX12DrawSetLight.h"
#include "DX12TextureBuffer.h"

void DX12DrawSetLight::Init(ID3D12Device* device)
{
	for (int i = 0; i < m_NumFrame; i++)
	{
		m_LightInfomations.push_back(std::make_unique<DX12UploadBuffer<DX12LightInfomation>>(device, 100, false));
	}

	CD3DX12_ROOT_PARAMETER baseRootParam[ROOT_COUNT];
	BaseRootParamSetting(baseRootParam);
	baseRootParam[LIGHTDATA_SRV].InitAsShaderResourceView(1,1);

	CD3DX12_ROOT_SIGNATURE_DESC rootDesc;
	rootDesc.Init(ROOT_COUNT, baseRootParam, _countof(m_StaticSamplers),
		m_StaticSamplers, D3D12_ROOT_SIGNATURE_FLAG_NONE);

	std::string textureNum = std::to_string(m_TextureBuffer->GetTexturesNum());
	D3D_SHADER_MACRO macros[] = {
		"MAXTEXTURE", textureNum.c_str(),
		NULL, NULL };

	D3D12_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	dsDesc.DepthFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;

	const char* dirLight = "DLight";

	D3D12_RASTERIZER_DESC rasterizer = {};
	rasterizer.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizer.CullMode = D3D12_CULL_MODE_BACK;

	m_PSOA.primitive = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	m_PSOA.rootSig = dirLight;
	m_PSOA.depthStencil = dirLight;
	m_PSOA.rasterizer = dirLight;
	m_PSOA.vs = dirLight;
	m_PSOA.ps = dirLight;
	m_PSOA.gs = dirLight;
	m_PSOCon->AddRootSignature(dirLight, rootDesc);
	m_PSOCon->AddDepthStencil(dirLight, dsDesc);
	m_PSOCon->AddRasterizer(dirLight, rasterizer);
	m_PSOCon->AddShader(dirLight, DX12_SHADER_VERTEX, L"../Common/MainShaders/DirectionalLightShader.hlsl", macros, "VS");
	m_PSOCon->AddShader(dirLight, DX12_SHADER_PIXEL, L"../Common/MainShaders/DirectionalLightShader.hlsl", macros, "PS");
	m_PSOCon->AddShader(dirLight, DX12_SHADER_GEOMETRY, L"../Common/MainShaders/DirectionalLightShader.hlsl", macros, "GS");
	
	const char* pointLight = "PLight";
	m_PointLightPSOA = m_PSOA;

	m_PointLightPSOA.primitive = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
	m_PointLightPSOA.depthStencil = pointLight;
	m_PointLightPSOA.gs = "";
	m_PointLightPSOA.vs = pointLight;
	m_PointLightPSOA.hs = pointLight;
	m_PointLightPSOA.ds = pointLight;
	m_PointLightPSOA.ps = pointLight;
	dsDesc.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	m_PSOCon->AddDepthStencil(pointLight, dsDesc);
	m_PSOCon->AddShader(pointLight, DX12_SHADER_VERTEX, L"../Common/MainShaders/PointLightShader.hlsl", macros, "VS");
	m_PSOCon->AddShader(pointLight, DX12_SHADER_PIXEL, L"../Common/MainShaders/PointLightShader.hlsl", macros, "PS");
	m_PSOCon->AddShader(pointLight, DX12_SHADER_HULL, L"../Common/MainShaders/PointLightShader.hlsl", macros, "HS");
	m_PSOCon->AddShader(pointLight, DX12_SHADER_DOMAIN, L"../Common/MainShaders/PointLightShader.hlsl", macros, "DS");

}

void DX12DrawSetLight::Draw(ID3D12GraphicsCommandList* cmd, const DX12PSOAttributeNames*, const DX12_COMPUTE_CULLING_DESC* culling)
{
	if (m_RenderCount)
	{
		////////////////////////////////////////////////////////////////////////////////////
		m_PSOCon->SetPSOToCommnadList(cmd, m_PSOA);
		SetBaseRoots(cmd);

		auto LightInfoSrv = m_LightInfomations[m_CurrFrame]->Resource()->GetGPUVirtualAddress();
		auto LightInfoSize = m_LightInfomations[m_CurrFrame]->GetElementByteSize();

		cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
		cmd->IASetVertexBuffers(0, 0, nullptr);
		
		cmd->SetGraphicsRootShaderResourceView(LIGHTDATA_SRV, LightInfoSrv);
		cmd->DrawInstanced(m_RenderCount[LIGHT_TYPE_DIRECTIONAL], 1, 0, 0);

		m_PSOCon->SetPSOToCommnadList(cmd, m_PointLightPSOA);
		cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);
		cmd->SetGraphicsRootShaderResourceView(LIGHTDATA_SRV, LightInfoSrv + LightInfoSize* PointLightBaseIndex);
		cmd->DrawInstanced(m_RenderCount[LIGHT_TYPE_POINT], 1, 0, 0);
	}
}

void DX12DrawSetLight::ReserveRender(const RenderInfo& info)
{
	unsigned int targetIndex = 0;
	DX12LightInfomation data;
	data.falloffAndPower = physx::PxVec4(info.lightInfo.falloffAndPower, 1);
	data.lightColor = physx::PxVec4(info.lightInfo.color, 1);
	data.posnAngle = physx::PxVec4(info.world.getPosition(), info.lightInfo.angle);
	data.dir = physx::PxVec4(info.world.rotate(physx::PxVec3(0, 0, 1)),1);

	switch (info.lightInfo.type)
	{
	case LIGHT_TYPE::LIGHT_TYPE_DIRECTIONAL:
		targetIndex = m_RenderCount[info.lightInfo.type];
		assert(m_RenderCount[info.lightInfo.type] < PointLightBaseIndex);
		break;
	case LIGHT_TYPE::LIGHT_TYPE_POINT:
		targetIndex = m_RenderCount[info.lightInfo.type] + PointLightBaseIndex;
		assert(m_RenderCount[info.lightInfo.type] < SpotLightBaseIndex- PointLightBaseIndex);
		break;
	case LIGHT_TYPE::LIGHT_TYPE_SPOT:
		targetIndex = m_RenderCount[info.lightInfo.type] + SpotLightBaseIndex;
		assert(m_RenderCount[info.lightInfo.type] < 100 - SpotLightBaseIndex);
		break;
	default:
		return;
		break;
	}

	m_LightInfomations[m_CurrFrame]->CopyData(targetIndex, data);
	m_RenderCount[info.lightInfo.type]++;
}

void DX12DrawSetLight::UpdateFrameCountAndClearWork()
{
	DX12DrawSet::UpdateFrameCountAndClearWork();
	
	for (auto& it : m_RenderCount)
	{
		it = 0;
	}
}
