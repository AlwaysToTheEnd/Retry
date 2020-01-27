#pragma once
#include <memory>
#include "DX12DrawSet.h"
#include "DX12UploadBuffer.h"
#include "DX12RenderClasses.h"

class DX12DrawSetLight : public DX12DrawSet
{
	enum
	{
		LIGHTDATA_SRV = BASE_ROOT_PARAM_COUNT,
		ROOT_COUNT
	};

	const char* pointSpotSignatureName = "pointSpot";
	unsigned int PointLightBaseIndex = 10;
	unsigned int SpotLightBaseIndex = 50;

public:
	DX12DrawSetLight(unsigned int numFrameResource,
		DX12PSOController* psoCon,
		DX12TextureBuffer* textureBuffer,
		const std::vector<DXGI_FORMAT>& rtvFormats,
		DXGI_FORMAT dsvFormat)
		: DX12DrawSet(numFrameResource, psoCon, textureBuffer, rtvFormats, dsvFormat)
	{

	}
	virtual ~DX12DrawSetLight() = default;

	virtual void	Init(ID3D12Device* device) override;
	virtual void	Draw(ID3D12GraphicsCommandList* cmd, const DX12PSOAttributeNames* = nullptr, const DX12_COMPUTE_CULLING_DESC* culling = nullptr) override;
	virtual void	ReserveRender(const RenderInfo& info) override;
	virtual void	UpdateFrameCountAndClearWork() override;

private:
	std::vector<std::unique_ptr<DX12UploadBuffer<DX12LightInfomation>>>		m_LightInfomations;

	DX12PSOAttributeNames	m_PointLightPSOA;
	DX12PSOAttributeNames	m_SpotLightPSOA;
	unsigned int			m_RenderCount[LIGHT_TYPE_COUNT] = {};
};