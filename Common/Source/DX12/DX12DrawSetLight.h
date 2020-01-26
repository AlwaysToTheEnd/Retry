#pragma once
#include <memory>
#include "DX12DrawSet.h"
#include "DX12UploadBuffer.h"
#include "DX12RenderClasses.h"

class DX12DrawSetLight : public DX12DrawSet
{
	enum
	{
		LIGHTTYPE_CONST = BASE_ROOT_PARAM_COUNT,
		ROOT_COUNT
	};

public:
	DX12DrawSetLight(unsigned int numFrameResource,
		DX12PSOController* psoCon,
		DX12TextureBuffer* textureBuffer,
		const std::vector<DXGI_FORMAT>& rtvFormats,
		DXGI_FORMAT dsvFormat)
		: DX12DrawSet(numFrameResource, psoCon, textureBuffer, rtvFormats, dsvFormat)
		, m_RenderCount(0)
	{

	}
	virtual ~DX12DrawSetLight() = default;

	virtual void	Init(ID3D12Device* device) override;
	virtual void	Draw(ID3D12GraphicsCommandList* cmd, const DX12PSOAttributeNames* custom = nullptr, const DX12_COMPUTE_CULLING_DESC* culling = nullptr) override;
	virtual void	ReserveRender(const RenderInfo& info) override;
	virtual void	UpdateFrameCountAndClearWork() override;

private:
	std::vector<std::unique_ptr<DX12UploadBuffer<DX12LightInfomation>>>		m_LightInfomations;
	unsigned int															m_RenderCount;
};