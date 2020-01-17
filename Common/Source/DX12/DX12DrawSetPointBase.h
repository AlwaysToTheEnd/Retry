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
		P1_OBJECT_SRV,
		P1_PASS_CB,
		P1_TEXTURE_TABLE,
		P1_ROOT_COUNT
	};

	struct PointBaseFrameResource
	{
		std::unique_ptr<DX12UploadBuffer<OnlyTexObjectConstants>> SRV;
		std::unique_ptr<DX12UploadBuffer<PointBaseVertex>> VB;
	};

public:
	DX12DrawSetPointBase(unsigned int numFrameResource)
		: DX12DrawSet(numFrameResource)
		, m_NumRenderPointObjects(0)
	{

	}
	virtual ~DX12DrawSetPointBase() = default;

	virtual void	Init(ID3D12Device* device, PSOController* psoCon,
		DXGI_FORMAT rtvFormat, DXGI_FORMAT dsvFormat,
		DX12TextureBuffer* textureBuffer,
		DX12IndexManagementBuffer<Material>* material, ID3D12Resource* mainPass) override;
	virtual void	Draw(ID3D12GraphicsCommandList* cmd, const PSOAttributeNames* custom = nullptr) override;
	virtual void	ReserveRender(const RenderInfo& info) override;

private:

private:
	std::vector<PointBaseFrameResource>	m_FrameResource;
	unsigned int						m_NumRenderPointObjects;

};