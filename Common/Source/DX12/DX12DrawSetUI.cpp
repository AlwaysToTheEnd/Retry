#include "DX12DrawSetUI.h"
#include "DX12TextureBuffer.h"

void DX12DrawSetUI::Init(ID3D12Device* device)
{
	m_UIPass = std::make_unique<DX12UploadBuffer<CGH::GlobalOptions::UIOption>>(device, m_NumFrame, true);
	m_VBs.resize(m_NumFrame);
	for (int i = 0; i < m_NumFrame; i++)
	{
		m_VBs[i] = std::make_unique<DX12UploadBuffer<DX12UIInfomation>>(device, 100, false);
	}

	CD3DX12_ROOT_PARAMETER uiRenderRootparam[UI_ROOT_COUNT];
	BaseRootParamSetting(uiRenderRootparam);
	uiRenderRootparam[UI_UIPASS_CB].InitAsConstantBufferView(1);

	CD3DX12_ROOT_SIGNATURE_DESC uiRenderRootDesc;
	uiRenderRootDesc.Init(UI_ROOT_COUNT, uiRenderRootparam, _countof(m_StaticSamplers),
		m_StaticSamplers, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	std::string textureNum = std::to_string(m_TextureBuffer->GetTexturesNum());
	D3D_SHADER_MACRO macros[] = {
		"MAXTEXTURE", textureNum.c_str(),
		NULL, NULL };

	m_PSOA.primitive = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	m_PSOA.rootSig = "UI";
	m_PSOA.vs = "UI";
	m_PSOA.ps = "UI";
	m_PSOA.gs = "UI";
	m_PSOCon->AddRootSignature("UI", uiRenderRootDesc);
	m_PSOCon->AddShader("UI", DX12_SHADER_VERTEX, L"../Common/MainShaders/UIShader.hlsl", macros, "VS");
	m_PSOCon->AddShader("UI", DX12_SHADER_PIXEL, L"../Common/MainShaders/UIShader.hlsl", macros, "PS");
	m_PSOCon->AddShader("UI", DX12_SHADER_GEOMETRY, L"../Common/MainShaders/UIShader.hlsl", macros, "GS");

	m_PSOA.input = "UI";
	m_PSOCon->AddInputLayout("UI",
		{
			{ "UICOLOR" ,0, DXGI_FORMAT_R32G32B32A32_FLOAT,0,0,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
			{ "UIPOS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "UISIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "UITYPE", 0, DXGI_FORMAT_R32_SINT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "UITEXTURE", 0, DXGI_FORMAT_R32_SINT, 0, 40, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		});
}

void DX12DrawSetUI::Draw(ID3D12GraphicsCommandList* cmd, const DX12PSOAttributeNames* custom)
{
	if (m_NumRenderUIs)
	{
		SetPSO(cmd, custom);

		SetBaseRoots(cmd);;

		static unsigned int uiPassStride = (sizeof(CGH::GlobalOptions::UIOption) + 255) & ~255;

		cmd->SetGraphicsRootConstantBufferView(UI_UIPASS_CB, m_UIPass->Resource()->GetGPUVirtualAddress() + (m_CurrFrame * uiPassStride));

		D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};

		vertexBufferView.BufferLocation = m_VBs[m_CurrFrame]->Resource()->GetGPUVirtualAddress();
		vertexBufferView.SizeInBytes = m_VBs[m_CurrFrame]->GetBufferSize();
		vertexBufferView.StrideInBytes = m_VBs[m_CurrFrame]->GetElementByteSize();

		cmd->IASetVertexBuffers(0, 1, &vertexBufferView);
		cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

		cmd->DrawInstanced(m_NumRenderUIs, 1, 0, 0);
	}
}

void DX12DrawSetUI::ReserveRender(const RenderInfo& info)
{
	DX12UIInfomation temp;
	temp.uiType = info.uiInfo.uiType;
	temp.size = info.uiInfo.size;
	temp.pos = info.world.getPosition();

	if (info.meshOrTextureName.size())
	{
		temp.textureIndex = m_TextureBuffer->GetTextureIndex(info.meshOrTextureName);
	}
	else
	{
		temp.color = info.uiInfo.color;
	}

	m_VBs[m_CurrFrame]->CopyData(m_NumRenderUIs, &temp);
	m_NumRenderUIs++;
}

void DX12DrawSetUI::UpdateFrameCountAndClearWork()
{
	DX12DrawSet::UpdateFrameCountAndClearWork();
	m_NumRenderUIs = 0;
}

void DX12DrawSetUI::UpdateUIPassCB(const CGH::GlobalOptions::UIOption& uiPass)
{
	m_UIPass->CopyData(m_CurrFrame, uiPass);
}
