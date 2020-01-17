#include "DX12DrawSetPointBase.h"
#include "DX12TextureBuffer.h"

void DX12DrawSetPointBase::Init(ID3D12Device* device, PSOController* psoCon, DXGI_FORMAT rtvFormat, DXGI_FORMAT dsvFormat, DX12TextureBuffer* textureBuffer, DX12IndexManagementBuffer<Material>* material, ID3D12Resource* mainPass)
{
	for (int i = 0; i < m_NumFrame; i++)
	{
		m_FrameResource.emplace_back();
		m_FrameResource.back().SRV = std::make_unique<DX12UploadBuffer<OnlyTexObjectConstants>>(device, 100, false);
		m_FrameResource.back().VB = std::make_unique<DX12UploadBuffer<PointBaseVertex>>(device, 100, false);
	}

	m_PSOA.rtvFormats.push_back(rtvFormat);
	m_PSOA.dsvFormat = dsvFormat;
	m_TextureBuffer = textureBuffer;
	m_MaterialBuffer = material;
	m_MainPassCB = mainPass;
	m_PSOCon = psoCon;

	CD3DX12_DESCRIPTOR_RANGE texTable;
	texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, m_TextureBuffer->GetTexturesNum(), 0);

	CD3DX12_ROOT_PARAMETER pointRenderRootParam[P1_ROOT_COUNT];
	pointRenderRootParam[P1_OBJECT_SRV].InitAsShaderResourceView(0, 1);
	pointRenderRootParam[P1_PASS_CB].InitAsConstantBufferView(0);
	pointRenderRootParam[P1_TEXTURE_TABLE].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);

	CD3DX12_ROOT_SIGNATURE_DESC pointRenderrootDesc;
	pointRenderrootDesc.Init(P1_ROOT_COUNT, pointRenderRootParam, _countof(m_StaticSamplers),
		m_StaticSamplers, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	std::string textureNum = std::to_string(m_TextureBuffer->GetTexturesNum());
	D3D_SHADER_MACRO macros[] = {
		"MAXTEXTURE", textureNum.c_str(),
		NULL, NULL };

	m_PSOA.primitive = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	m_PSOA.rootSig = "P1";
	m_PSOA.vs = "P1";
	m_PSOA.ps = "P1";
	m_PSOA.gs = "P1";
	m_PSOCon->AddRootSignature("P1", pointRenderrootDesc);
	m_PSOCon->AddShader("P1", DX12_SHADER_VERTEX, L"../Common/MainShaders/pointShader.hlsl", macros, "VS");
	m_PSOCon->AddShader("P1", DX12_SHADER_PIXEL, L"../Common/MainShaders/pointShader.hlsl", macros, "PS");
	m_PSOCon->AddShader("P1", DX12_SHADER_GEOMETRY, L"../Common/MainShaders/pointShader.hlsl", macros, "GS");

	m_PSOA.input = "P1";
	m_PSOCon->AddInputLayout("P1",
		{
			{ "MESHTYPE", 0, DXGI_FORMAT_R32_UINT,0,0,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
			{ "CBINDEX", 0, DXGI_FORMAT_R32_UINT, 0, 4, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "MESHSIZE", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 8, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "MESHCOLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		});
}

void DX12DrawSetPointBase::Draw(ID3D12GraphicsCommandList* cmd, const PSOAttributeNames* custom)
{
	if (m_NumRenderPointObjects)
	{
		SetPSO(cmd, custom);

		auto& currFrameSource = m_FrameResource[m_CurrFrame];

		ID3D12DescriptorHeap* descriptorHeaps[] = { m_TextureBuffer->GetHeap() };
		cmd->SetDescriptorHeaps(1, descriptorHeaps);

		cmd->SetGraphicsRootShaderResourceView(P1_OBJECT_SRV, currFrameSource.SRV->Resource()->GetGPUVirtualAddress());
		cmd->SetGraphicsRootConstantBufferView(P1_PASS_CB, GetCurrMainPassAddress());
		cmd->SetGraphicsRootDescriptorTable(P1_TEXTURE_TABLE, m_TextureBuffer->GetHeap()->GetGPUDescriptorHandleForHeapStart());

		D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};

		vertexBufferView.BufferLocation = currFrameSource.VB->Resource()->GetGPUVirtualAddress();
		vertexBufferView.SizeInBytes = currFrameSource.VB->GetBufferSize();
		vertexBufferView.StrideInBytes = currFrameSource.VB->GetElementByteSize();

		cmd->IASetVertexBuffers(0, 1, &vertexBufferView);
		cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

		cmd->DrawInstanced(m_NumRenderPointObjects, 1, 0, 0);

		m_NumRenderPointObjects = 0;
	}
}

void DX12DrawSetPointBase::ReserveRender(const RenderInfo& info)
{
	PointBaseVertex temp;
	OnlyTexObjectConstants OTObjectConstnat;
	auto& currFrameSource = m_FrameResource[m_CurrFrame];

	temp.type = info.type;
	temp.cbIndex = m_NumRenderPointObjects;
	temp.size = info.point.size;

	OTObjectConstnat.world = info.world;

	if (info.meshOrTextureName.size())
	{
		OTObjectConstnat.textureIndex = m_TextureBuffer->GetTextureIndex(info.meshOrTextureName);
	}

	if (OTObjectConstnat.textureIndex == -1)
	{
		temp.color = info.point.color;
	}

	currFrameSource.VB->CopyData(m_NumRenderPointObjects, &temp);
	currFrameSource.SRV->CopyData(m_NumRenderPointObjects, &OTObjectConstnat);
	m_NumRenderPointObjects++;
}
