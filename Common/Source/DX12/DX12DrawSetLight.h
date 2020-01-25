#pragma once
#include "DX12DrawSet.h"

class DX12DrawSetLight : public DX12DrawSet
{
	enum
	{
		OBJECT_CB = BASE_ROOT_PARAM_COUNT,
		ROOT_COUNT
	};

public:
	DX12DrawSetLight(unsigned int numFrameResource,
		PSOController* psoCon,
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
	unsigned int	m_RenderCount;
};