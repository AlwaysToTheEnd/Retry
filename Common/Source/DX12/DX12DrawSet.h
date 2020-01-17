#pragma once
#include <d3d12.h>
#include <memory>
#include <vector>
#include <dxgiformat.h>
#include <string>
#include "GraphicBase.h"
#include "PSOController.h"
#include "DX12IndexManagementBuffer.h"

//struct DX12_TARGET_SHADER
//{
//	std::string						shaderFilePath;
//	std::vector<std::string>		entryPoints;
//	std::vector<DX12_SHADER_TYPE>	types;
//	std::vector<D3D_SHADER_MACRO>	macros;
//};

struct PSOAttributeNames
{
	std::vector<DXGI_FORMAT>		rtvFormats;
	DXGI_FORMAT						dsvFormat;
	D3D12_PRIMITIVE_TOPOLOGY_TYPE	primitive = D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
	std::string						input = "";
	std::string						rootSig = "";
	std::string						rasterizer = "";
	std::string						blend = "";
	std::string						depthStencil = "";
	std::string						vs = "";
	std::string						ps = "";
	std::string						gs = "";
	std::string						hs = "";
	std::string						ds = "";
};

class DX12TextureBuffer;

class DX12DrawSet
{
public:
	DX12DrawSet(unsigned int numFrameResource)
		:m_NumFrame(numFrameResource)
	{

	}
	virtual ~DX12DrawSet() = default;

	virtual void	Init(ID3D12Device* device, PSOController* psoCon, 
						 DXGI_FORMAT rtvFormat, DXGI_FORMAT dsvFormat,
						 DX12TextureBuffer* textureBuffer, 
						 DX12IndexManagementBuffer<Material>* material, ID3D12Resource* mainPass) = 0;
	virtual void	Draw(ID3D12GraphicsCommandList* cmd, const PSOAttributeNames* custom=nullptr) = 0;
	virtual void	ReserveRender(const RenderInfo& info) = 0;
	virtual	void	UploadBuffersClear() = 0;

	void						UpdateFrameCount();
	D3D12_GPU_VIRTUAL_ADDRESS	GetCurrMainPassAddress() const;
	void						SetPSO(ID3D12GraphicsCommandList* cmd, const PSOAttributeNames* custom);

private:
	void						AttributeSetToPSO(ID3D12GraphicsCommandList* cmd, const PSOAttributeNames& custom);

protected:
	static D3D12_STATIC_SAMPLER_DESC		m_StaticSamplers[7];
	const unsigned int						m_NumFrame;
	unsigned int							m_CurrFrame = 0;
	PSOAttributeNames						m_PSOA;
	PSOController*							m_PSOCon = nullptr;

	ID3D12Resource*							m_MainPassCB = nullptr;
	DX12IndexManagementBuffer<Material>*	m_MaterialBuffer = nullptr;
	DX12TextureBuffer*						m_TextureBuffer = nullptr;
};