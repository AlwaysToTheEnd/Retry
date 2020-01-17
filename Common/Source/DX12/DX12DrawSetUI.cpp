#include "DX12DrawSetUI.h"
#include "DX12TextureBuffer.h"

void DX12DrawSetUI::Init(ID3D12Device* device, PSOController* psoCon, DXGI_FORMAT rtvFormat, DXGI_FORMAT dsvFormat, DX12TextureBuffer* textureBuffer, DX12IndexManagementBuffer<Material>* material, ID3D12Resource* mainPass)
{
	m_UIPass = std::make_unique<DX12UploadBuffer<CGH::GlobalOptions::UIOption>>(device, m_NumFrame, true);
	m_VBs.resize(m_NumFrame);
	for (int i = 0; i < m_NumFrame; i++)
	{
		m_VBs[i] = std::make_unique<DX12UploadBuffer<UIInfomation>>(device, 100, false);
	}

	m_PSOA.rtvFormats.push_back(rtvFormat);
	m_PSOA.dsvFormat = dsvFormat;
	m_TextureBuffer = textureBuffer;
	m_MaterialBuffer = material;
	m_MainPassCB = mainPass;
	m_PSOCon = psoCon;

	CD3DX12_DESCRIPTOR_RANGE texTable;
	texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, m_TextureBuffer->GetTexturesNum(), 0);

	CD3DX12_ROOT_PARAMETER uiRenderRootparam[UI_ROOT_COUNT];
	uiRenderRootparam[UI_PASS_CB].InitAsConstantBufferView(0);
	uiRenderRootparam[UI_UIPASS_CB].InitAsConstantBufferView(1);
	uiRenderRootparam[UI_TEXTURE_TABLE].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);

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

void DX12DrawSetUI::Draw(ID3D12GraphicsCommandList* cmd, const PSOAttributeNames* custom)
{
	if (m_NumRenderUIs)
	{
		SetPSO(cmd, custom);

		ID3D12DescriptorHeap* descriptorHeaps[] = { m_TextureBuffer->GetHeap() };
		cmd->SetDescriptorHeaps(1, descriptorHeaps);

		static unsigned int uiPassStride = (sizeof(CGH::GlobalOptions::UIOption) + 255) & ~255;

		cmd->SetGraphicsRootConstantBufferView(UI_PASS_CB, GetCurrMainPassAddress());
		cmd->SetGraphicsRootConstantBufferView(UI_UIPASS_CB, m_UIPass->Resource()->GetGPUVirtualAddress() + (m_CurrFrame * uiPassStride));
		cmd->SetGraphicsRootDescriptorTable(UI_TEXTURE_TABLE, m_TextureBuffer->GetHeap()->GetGPUDescriptorHandleForHeapStart());

		D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};

		vertexBufferView.BufferLocation = m_VBs[m_CurrFrame]->Resource()->GetGPUVirtualAddress();
		vertexBufferView.SizeInBytes = m_VBs[m_CurrFrame]->GetBufferSize();
		vertexBufferView.StrideInBytes = m_VBs[m_CurrFrame]->GetElementByteSize();

		cmd->IASetVertexBuffers(0, 1, &vertexBufferView);
		cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

		cmd->DrawInstanced(m_NumRenderUIs, 1, 0, 0);
		m_NumRenderUIs = 0;
	}
}

void DX12DrawSetUI::ReserveRender(const RenderInfo& info)
{
	UIInfomation temp;
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

void DX12DrawSetUI::UpdateUIPassCB(const CGH::GlobalOptions::UIOption& uiPass)
{
	m_UIPass->CopyData(m_CurrFrame, uiPass);
}
