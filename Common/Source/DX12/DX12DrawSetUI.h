#pragma once
#include "Vertex.h"
#include "DX12DrawSet.h"
#include "DX12RenderClasses.h"
#include "DX12DefaultBuffer.h"
#include "DX12UploadBuffer.h"

class DX12DrawSetUI :public DX12DrawSet
{
	enum
	{
		UI_PASS_CB,
		UI_UIPASS_CB,
		UI_TEXTURE_TABLE,
		UI_ROOT_COUNT
	};

public:
	DX12DrawSetUI(unsigned int numFrameResource)
		: DX12DrawSet(numFrameResource)
		, m_NumRenderUIs(0)
	{

	}
	virtual ~DX12DrawSetUI() = default;

	virtual void	Init(ID3D12Device* device, PSOController* psoCon,
							DXGI_FORMAT rtvFormat, DXGI_FORMAT dsvFormat,
							DX12TextureBuffer* textureBuffer,
							DX12IndexManagementBuffer<Material>* material, ID3D12Resource* mainPass) override;
	virtual void	Draw(ID3D12GraphicsCommandList* cmd) override;
	virtual void	ReserveRender(const RenderInfo& info) override;
	virtual	void	UploadBuffersClear() override {}

	void UpdateUIPassCB(const CGH::GlobalOptions::UIOption& uiPass);

private:
	std::unique_ptr<DX12UploadBuffer<CGH::GlobalOptions::UIOption>> m_UIPass;
	std::vector<std::unique_ptr<DX12UploadBuffer<UIInfomation>>>	m_VBs;

	unsigned int													m_NumRenderUIs;
};