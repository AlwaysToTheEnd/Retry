#pragma once
#include "Vertex.h"
#include "DX12DrawSet.h"
#include "DX12RenderClasses.h"
#include "DX12DefaultBuffer.h"
#include "DX12UploadBuffer.h"

class DX12DrawSetPointBase :public DX12DrawSet
{
	enum
	{
		P1_OBJECT_SRV = BASE_ROOT_PARAM_COUNT,
		P1_ROOT_COUNT
	};

	struct PointBaseFrameResource
	{
		std::unique_ptr<DX12UploadBuffer<DX12OnlyTexObjectConstants>> SRV;
		std::unique_ptr<DX12UploadBuffer<PointBaseVertex>> VB;
	};

public:
	DX12DrawSetPointBase(unsigned int numFrameResource,
		DX12PSOController* psoCon,
		DX12TextureBuffer* textureBuffer,
		const std::vector<DXGI_FORMAT>& rtvFormats,
		DXGI_FORMAT dsvFormat)
		: DX12DrawSet(numFrameResource, psoCon, textureBuffer, rtvFormats, dsvFormat)
		, m_NumRenderPointObjects(0)
	{

	}
	virtual ~DX12DrawSetPointBase() = default;

	virtual void	Init(ID3D12Device* device) override;
	virtual void	Draw(ID3D12GraphicsCommandList* cmd, const DX12PSOAttributeNames* custom = nullptr, const DX12_COMPUTE_CULLING_DESC* culling = nullptr) override;
	virtual void	ReserveRender(const RenderInfo& info) override;
	virtual void	UpdateFrameCountAndClearWork() override;

private:

private:
	std::vector<PointBaseFrameResource>	m_FrameResource;
	unsigned int						m_NumRenderPointObjects;

};