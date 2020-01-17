#pragma once
#include "Vertex.h"
#include "DX12DrawSet.h"
#include "DX12RenderClasses.h"
#include "DX12DefaultBuffer.h"
#include "DX12UploadBuffer.h"

class DX12DrawSetHeightField :public DX12DrawSet
{
	enum
	{
		PASS_CB,
		TEXTURE_TABLE,
		MATERIAL_SRV,
		ROOT_COUNT
	};

public:
	DX12DrawSetHeightField(unsigned int numFrameResource)
		: DX12DrawSet(numFrameResource)
	{

	}
	virtual ~DX12DrawSetHeightField() = default;

	virtual void	Init(ID3D12Device* device, PSOController* psoCon,
		DXGI_FORMAT rtvFormat, DXGI_FORMAT dsvFormat,
		DX12TextureBuffer* textureBuffer,
		DX12IndexManagementBuffer<Material>* material, ID3D12Resource* mainPass) override;
	virtual void	Draw(ID3D12GraphicsCommandList* cmd, const PSOAttributeNames* custom = nullptr) override;
	virtual void	ReserveRender(const RenderInfo& info) override;
	virtual	void	UploadBuffersClear() override {}

private:

};