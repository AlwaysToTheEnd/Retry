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
		UI_UIPASS_CB = BASE_ROOT_PARAM_COUNT,
		UI_ROOT_COUNT
	};

public:
	DX12DrawSetUI(unsigned int numFrameResource,
		PSOController* psoCon,
		DX12TextureBuffer* textureBuffer,
		DX12IndexManagementBuffer<Material>* material,
		const std::vector<DXGI_FORMAT>& rtvFormats,
		DXGI_FORMAT dsvFormat)
		: DX12DrawSet(numFrameResource, psoCon, textureBuffer, material, rtvFormats, dsvFormat)
		, m_NumRenderUIs(0)
	{

	}
	virtual ~DX12DrawSetUI() = default;

	virtual void	Init(ID3D12Device* device) override;
	virtual void	Draw(ID3D12GraphicsCommandList* cmd, const DX12PSOAttributeNames* custom = nullptr) override;
	virtual void	ReserveRender(const RenderInfo& info) override;
	virtual void	UpdateFrameCountAndClearWork() override;

	void UpdateUIPassCB(const CGH::GlobalOptions::UIOption& uiPass);

private:
	std::unique_ptr<DX12UploadBuffer<CGH::GlobalOptions::UIOption>>		m_UIPass;
	std::vector<std::unique_ptr<DX12UploadBuffer<DX12UIInfomation>>>	m_VBs;

	unsigned int														m_NumRenderUIs;
};