#pragma once
#include <d3d12.h>
#include <d3dx12.h>
#include <memory>
#include <vector>
#include <dxgiformat.h>
#include <string>
#include "GraphicBase.h"
#include "DX12PSOController.h"
#include "DX12IndexManagementBuffer.h"
#include "DX12MeshComputeCulling.h"

//struct DX12_TARGET_SHADER
//{
//	std::string						shaderFilePath;
//	std::vector<std::string>		entryPoints;
//	std::vector<DX12_SHADER_TYPE>	types;
//	std::vector<D3D_SHADER_MACRO>	macros;
//};

struct CD3DX12_ROOT_PARAMETER;
class DX12TextureBuffer;
class DX12SwapChain;

class DX12DrawSet
{
public:
	static const std::string ShadowMapShaderCallName;

protected:
	enum BASE_ROOT_PARAM
	{
		PASS_CB,
		MATERIAL_SRV,
		TEXTURE_TABLE,
		BASE_ROOT_PARAM_COUNT
	};
	
public:
	DX12DrawSet(unsigned int numFrameResource, 
		DX12PSOController* psoCon,
		DX12TextureBuffer* textureBuffer, 
		const std::vector<DXGI_FORMAT>& rtvFormats,
		DXGI_FORMAT dsvFormat)
		:m_NumFrame(numFrameResource)
		,m_PSOCon(psoCon)
		,m_TextureBuffer(textureBuffer)
	{
		m_PSOA.rtvFormats = rtvFormats;
		m_PSOA.dsvFormat = dsvFormat;
		m_Draws.push_back(this);
	}
	virtual ~DX12DrawSet() = default;

	virtual void	Init(ID3D12Device* device) = 0;
	virtual void	Draw(ID3D12GraphicsCommandList* cmd, const DX12PSOAttributeNames* custom = nullptr, const DX12_COMPUTE_CULLING_DESC* culling = nullptr) = 0;
	virtual void	ReserveRender(const RenderInfo& info) = 0;
	virtual void	UpdateFrameCountAndClearWork();

	void			SetPSO(ID3D12GraphicsCommandList* cmd, const DX12PSOAttributeNames* custom);
	static void		AllDrawsFrameCountAndClearWork();
	static void		SetBaseResource(ID3D12Resource* mainPass, DX12IndexManagementBuffer<Material>* material, DX12SwapChain* swapChain);

protected:
	void				BaseRootParamSetting(CD3DX12_ROOT_PARAMETER params[BASE_ROOT_PARAM_COUNT]);
	void				SetBaseRoots(ID3D12GraphicsCommandList* cmd);
	virtual std::string GetShadowRenderShaderName(DX12_SHADER_TYPE type) { return ""; }

protected:
	static D3D12_STATIC_SAMPLER_DESC			m_StaticSamplers[7];
	static std::vector<DX12DrawSet*>			m_Draws;
	static DX12IndexManagementBuffer<Material>* m_MaterialBuffer;
	static ID3D12Resource*						m_MainPassCB;
	static DX12SwapChain*						m_SwapChain;

	const unsigned int							m_NumFrame;
	unsigned int								m_CurrFrame = 0;
	DX12PSOAttributeNames						m_PSOA;
	DX12PSOController*							m_PSOCon = nullptr;

	DX12TextureBuffer*							m_TextureBuffer = nullptr;
};