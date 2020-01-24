#pragma once
#include "Vertex.h"
#include "DX12RenderClasses.h"
#include "DX12DrawSet.h"
#include "DX12MeshSet.h"

#pragma pack(push, 4)
struct DX12NormalMeshIndirectCommand
{
	D3D12_GPU_VIRTUAL_ADDRESS		cbv;
	D3D12_DRAW_INDEXED_ARGUMENTS	draw;
	int								pad;
};
#pragma pack(pop)

class DX12DrawSetNormalMesh :public DX12DrawSet
{
	enum
	{
		OBJECT_CB = BASE_ROOT_PARAM_COUNT,
		ROOT_COUNT
	};

public:
	DX12DrawSetNormalMesh(unsigned int numFrameResource,
		PSOController* psoCon,
		DX12TextureBuffer* textureBuffer,
		const std::vector<DXGI_FORMAT>& rtvFormats,
		DXGI_FORMAT dsvFormat, DX12MeshSet<Vertex>& meshSet)
		: DX12DrawSet(numFrameResource, psoCon, textureBuffer, rtvFormats, dsvFormat)
		, m_MeshSet(meshSet)
		, m_RenderCount(0)
	{

	}
	virtual ~DX12DrawSetNormalMesh() = default;

	virtual void	Init(ID3D12Device* device) override;
	virtual void	Draw(ID3D12GraphicsCommandList * cmd, const DX12PSOAttributeNames* custom = nullptr, const DX12_COMPUTE_CULLING_DESC* culling = nullptr) override;
	virtual void	ReserveRender(const RenderInfo& info) override;
	virtual void	UpdateFrameCountAndClearWork() override;

private:
	virtual std::string GetShadowRenderShaderName(DX12_SHADER_TYPE type) override;

private:
	FrameObjectCBs																	m_MeshObjectCB;
	std::vector<std::unique_ptr<DX12UploadBuffer<DX12NormalMeshIndirectCommand>>>	m_ReservedCommands;

	DX12MeshComputeCulling											m_Culling;
	DX12MeshSet<Vertex>&											m_MeshSet;
	unsigned int													m_RenderCount;
};