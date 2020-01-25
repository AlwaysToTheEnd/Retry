#include "DX12DrawSet.h"
#include "DX12RenderClasses.h"
#include "DX12TextureBuffer.h"
#include "DX12SwapChain.h"

std::vector<DX12DrawSet*> DX12DrawSet::m_Draws;
DX12IndexManagementBuffer<Material>* DX12DrawSet::m_MaterialBuffer = nullptr;
ID3D12Resource* DX12DrawSet::m_MainPassCB = nullptr;
const std::string DX12DrawSet::ShadowMapShaderCallName = "&_shadowRender";
DX12SwapChain* DX12DrawSet::m_SwapChain = nullptr;

D3D12_STATIC_SAMPLER_DESC DX12DrawSet::m_StaticSamplers[7] =
{
	CD3DX12_STATIC_SAMPLER_DESC(
	0,
	D3D12_FILTER_MIN_MAG_MIP_POINT,
	D3D12_TEXTURE_ADDRESS_MODE_WRAP,
	D3D12_TEXTURE_ADDRESS_MODE_WRAP,
	D3D12_TEXTURE_ADDRESS_MODE_WRAP),

	CD3DX12_STATIC_SAMPLER_DESC(
	1,
	D3D12_FILTER_MIN_MAG_MIP_POINT,
	D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
	D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
	D3D12_TEXTURE_ADDRESS_MODE_CLAMP),

	CD3DX12_STATIC_SAMPLER_DESC(
	2,
	D3D12_FILTER_MIN_MAG_MIP_LINEAR,
	D3D12_TEXTURE_ADDRESS_MODE_WRAP,
	D3D12_TEXTURE_ADDRESS_MODE_WRAP,
	D3D12_TEXTURE_ADDRESS_MODE_WRAP),

	CD3DX12_STATIC_SAMPLER_DESC(
	3,
	D3D12_FILTER_MIN_MAG_MIP_LINEAR,
	D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
	D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
	D3D12_TEXTURE_ADDRESS_MODE_CLAMP),

	CD3DX12_STATIC_SAMPLER_DESC(
	4,
	D3D12_FILTER_ANISOTROPIC,
	D3D12_TEXTURE_ADDRESS_MODE_WRAP,
	D3D12_TEXTURE_ADDRESS_MODE_WRAP,
	D3D12_TEXTURE_ADDRESS_MODE_WRAP,
	0.0f,
	8),

	CD3DX12_STATIC_SAMPLER_DESC(
	5,
	D3D12_FILTER_ANISOTROPIC,
	D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
	D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
	D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
	0.0f,
	8),

	 CD3DX12_STATIC_SAMPLER_DESC(
	6,
	D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT,
	D3D12_TEXTURE_ADDRESS_MODE_BORDER,
	D3D12_TEXTURE_ADDRESS_MODE_BORDER,
	D3D12_TEXTURE_ADDRESS_MODE_BORDER,
	0.0f,
	16,
	D3D12_COMPARISON_FUNC_LESS_EQUAL,
	D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK)
};

void DX12DrawSet::UpdateFrameCountAndClearWork()
{
	m_CurrFrame = (m_CurrFrame + 1) % m_NumFrame;
}

void DX12DrawSet::SetPSO(ID3D12GraphicsCommandList* cmd, const DX12PSOAttributeNames* custom)
{
	if (custom)
	{
		DX12PSOAttributeNames temp;

		temp.rtvFormats = custom->rtvFormats;
		temp.dsvFormat = custom->dsvFormat;

		temp.primitive = m_PSOA.primitive;
		temp.input = m_PSOA.input;
		temp.rootSig = m_PSOA.rootSig;
		
		if (custom->rasterizer.empty())
		{
			temp.rasterizer = m_PSOA.rasterizer;
		}

		if (custom->blend.empty())
		{
			temp.blend = m_PSOA.blend;
		}

		if (custom->depthStencil.empty())
		{
			temp.depthStencil = m_PSOA.depthStencil;
		}

		if (custom->vs.empty())
		{
			temp.vs = m_PSOA.vs;
		}
		else if(custom->vs == ShadowMapShaderCallName)
		{
			temp.vs = GetShadowRenderShaderName(DX12_SHADER_VERTEX);

			if (temp.vs.empty())
			{
				temp.vs = m_PSOA.vs;
			}
		}

		if (custom->ps.empty())
		{
			temp.ps = m_PSOA.ps;
		}
		else if (custom->ps == ShadowMapShaderCallName)
		{
			temp.ps = GetShadowRenderShaderName(DX12_SHADER_PIXEL);

			if (temp.ps.empty())
			{
				temp.ps = m_PSOA.ps;
			}
		}

		if (custom->gs.empty())
		{
			temp.gs = m_PSOA.gs;
		}

		if (custom->hs.empty())
		{
			temp.hs = m_PSOA.hs;
		}

		if (custom->ds.empty())
		{
			temp.ds = m_PSOA.ds;
		}

		AttributeSetToPSO(cmd, temp);
	}
	else
	{
		AttributeSetToPSO(cmd, m_PSOA);
	}
}

void DX12DrawSet::AllDrawsFrameCountAndClearWork()
{
	for (auto& it : m_Draws)
	{
		it->UpdateFrameCountAndClearWork();
	}
}

void DX12DrawSet::SetBaseResource(ID3D12Resource* mainPass, DX12IndexManagementBuffer<Material>* material, DX12SwapChain* swapChain)
{
	m_MainPassCB = mainPass;
	m_MaterialBuffer = material;
	m_SwapChain = swapChain;
}

void DX12DrawSet::BaseRootParamSetting(CD3DX12_ROOT_PARAMETER params[BASE_ROOT_PARAM_COUNT])
{
	static CD3DX12_DESCRIPTOR_RANGE targetTexTable = {};
	static CD3DX12_DESCRIPTOR_RANGE texTable = {};
	targetTexTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, DX12SwapChain::GBUFFER_RESOURCE_COUNT, 0);
	texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, m_TextureBuffer->GetTexturesNum(), DX12SwapChain::GBUFFER_RESOURCE_COUNT);

	params[PASS_CB].InitAsConstantBufferView(0);
	params[MATERIAL_SRV].InitAsShaderResourceView(0, 1);
	params[TARGETTEXTURE_TABLE].InitAsDescriptorTable(1, &targetTexTable, D3D12_SHADER_VISIBILITY_PIXEL);
	params[TEXTURE_TABLE].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);
}

void DX12DrawSet::SetBaseRoots(ID3D12GraphicsCommandList* cmd)
{
	ID3D12DescriptorHeap* targetTexHeap[] = {  m_SwapChain->GetSrvHeap() };
	ID3D12DescriptorHeap* textureHeap[] = { m_TextureBuffer->GetHeap() };
	cmd->SetDescriptorHeaps(1, targetTexHeap);
	cmd->SetGraphicsRootConstantBufferView(PASS_CB, m_MainPassCB->GetGPUVirtualAddress());
	cmd->SetGraphicsRootShaderResourceView(MATERIAL_SRV, m_MaterialBuffer->GetBufferResource()->GetGPUVirtualAddress());
	cmd->SetGraphicsRootDescriptorTable(TARGETTEXTURE_TABLE, m_SwapChain->CurrSRVsGPU());
	cmd->SetDescriptorHeaps(1, textureHeap);
	cmd->SetGraphicsRootDescriptorTable(TEXTURE_TABLE, m_TextureBuffer->GetHeap()->GetGPUDescriptorHandleForHeapStart());
}

void DX12DrawSet::AttributeSetToPSO(ID3D12GraphicsCommandList* cmd, const DX12PSOAttributeNames& custom)
{
	m_PSOCon->SetPSOToCommnadList(cmd, custom.rtvFormats, custom.dsvFormat,
		custom.primitive, custom.input, custom.rootSig, custom.rasterizer, custom.blend,
		custom.depthStencil, custom.vs, custom.ps, custom.gs, custom.hs, custom.ds);
}
