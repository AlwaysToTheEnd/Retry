#include "DX12DrawSetLight.h"
#include "DX12TextureBuffer.h"

void DX12DrawSetLight::Init(ID3D12Device* device)
{
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

	//m_PSOA.primitive = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//m_PSOA.rootSig = "normal";
	//m_PSOA.vs = "normal";
	//m_PSOA.ps = "normal";
	//m_PSOCon->AddRootSignature("normal", rootDesc);
	//m_PSOCon->AddShader("normal", DX12_SHADER_VERTEX, L"../Common/MainShaders/MeshShader.hlsl", macros, "VS");
	//m_PSOCon->AddShader("normal", DX12_SHADER_PIXEL, L"../Common/MainShaders/MeshShader.hlsl", macros, "PS");

	//m_PSOA.input = "normal";
	//m_PSOCon->AddInputLayout("normal",
	//	{
	//		{ "POSITION" ,0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
	//		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	//		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	//	});
}

void DX12DrawSetLight::Draw(ID3D12GraphicsCommandList* cmd, const DX12PSOAttributeNames* custom, const DX12_COMPUTE_CULLING_DESC* culling)
{

}

void DX12DrawSetLight::ReserveRender(const RenderInfo& info)
{
}

void DX12DrawSetLight::UpdateFrameCountAndClearWork()
{
}
