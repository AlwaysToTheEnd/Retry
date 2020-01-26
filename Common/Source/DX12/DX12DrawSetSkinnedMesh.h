#pragma once
#include "Vertex.h"
#include "DX12RenderClasses.h"
#include "DX12DrawSet.h"
#include "DX12MeshSet.h"
#include "DX12MeshComputeCulling.h"

#pragma pack(push, 4)
struct DX12SkinnedMeshIndirectCommand
{
	D3D12_GPU_VIRTUAL_ADDRESS		objectCbv;
	D3D12_GPU_VIRTUAL_ADDRESS		aniBoneCbv;
	D3D12_DRAW_INDEXED_ARGUMENTS	draw;
	int								pad[3];
};
#pragma pack(pop)

class DX12DrawSetSkinnedMesh : public DX12DrawSet
{
	enum
	{
		OBJECT_CB =BASE_ROOT_PARAM_COUNT,
		ANIBONE_CB,
		ROOT_COUNT
	};

public:
	DX12DrawSetSkinnedMesh(unsigned int numFrameResource,
		DX12PSOController* psoCon,
		DX12TextureBuffer* textureBuffer,
		const std::vector<DXGI_FORMAT>& rtvFormats,
		DXGI_FORMAT dsvFormat, DX12MeshSet<SkinnedVertex>& meshSet)
		: DX12DrawSet(numFrameResource, psoCon, textureBuffer, rtvFormats, dsvFormat)
		, m_MeshSet(meshSet)
		, m_RenderCount(0)
	{

	}
	virtual ~DX12DrawSetSkinnedMesh() = default;

	virtual void	Init(ID3D12Device * device) override;
	virtual void	Draw(ID3D12GraphicsCommandList * cmd, const DX12PSOAttributeNames* custom = nullptr, const DX12_COMPUTE_CULLING_DESC* culling = nullptr) override;
	virtual void	ReserveRender(const RenderInfo& info) override;
	virtual void	UpdateFrameCountAndClearWork() override;

	void			UpdateAniBoneCB(const std::vector<AniBoneMat>& reservedData);

private:
	virtual std::string GetShadowRenderShaderName(DX12_SHADER_TYPE type) override;

private:
	FrameObjectCBs																	m_MeshObjectCB;
	std::vector<std::unique_ptr<DX12UploadBuffer<AniBoneMat>>>						m_AniBoneCB;
	std::vector<std::unique_ptr<DX12UploadBuffer<DX12SkinnedMeshIndirectCommand>>>	m_ReservedCommands;

	DX12MeshComputeCulling			m_Culling;
	DX12MeshSet<SkinnedVertex>&		m_MeshSet;
	unsigned int					m_RenderCount;
};