#pragma once
#include "Vertex.h"
#include "DX12RenderClasses.h"
#include "DX12DrawSet.h"
#include "DX12MeshSet.h"
#include "DX12CommandBuffer.h"

struct DX12SkinnedMeshIndirectCommand
{
	D3D12_GPU_VIRTUAL_ADDRESS		objectCbv;
	D3D12_GPU_VIRTUAL_ADDRESS		aniBoneCbv;
	D3D12_DRAW_INDEXED_ARGUMENTS	draw;
};

class DX12DrawSetSkinnedMesh : public DX12DrawSet
{
	enum
	{
		OBJECT_CB =BASE_ROOT_PARAM_COUNT,
		ANIBONE_CB,
		ROOT_COUNT
	};

	enum
	{
		COMPUTE_OBJECTNUM_CONST,
		COMPUTE_OBJECTCB_SRV,
		COMPUTE_INPUTCOMMAND_SRV,
		COMPUTE_OUTCOMMAND_UAV,
		COMPUTE_ROOT_COUNT,
	};

public:
	DX12DrawSetSkinnedMesh(unsigned int numFrameResource,
		PSOController* psoCon,
		DX12TextureBuffer* textureBuffer,
		const std::vector<DXGI_FORMAT>& rtvFormats,
		DXGI_FORMAT dsvFormat, DX12MeshSet<SkinnedVertex>& meshSet)
		: DX12DrawSet(numFrameResource, psoCon, textureBuffer, rtvFormats, dsvFormat)
		, m_MeshSet(meshSet)
	{

	}
	virtual ~DX12DrawSetSkinnedMesh() = default;

	virtual void	Init(ID3D12Device * device) override;
	virtual void	Draw(ID3D12GraphicsCommandList * cmd, const DX12PSOAttributeNames* custom = nullptr) override;
	virtual void	ReserveRender(const RenderInfo& info) override;
	virtual void	UpdateFrameCountAndClearWork() override;

	void			UpdateAniBoneCB(const std::vector<AniBoneMat>& reservedData);


private:
	std::vector<std::unique_ptr<DX12UploadBuffer<DX12ObjectConstants>>>				m_MeshObjectCB;
	std::vector<std::unique_ptr<DX12UploadBuffer<AniBoneMat>>>						m_AniBoneCB;
	std::vector<std::unique_ptr<DX12CommandBuffer<DX12SkinnedMeshIndirectCommand>>>	m_Commands;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>									m_CommandUAVHeap;

	DX12MeshSet<SkinnedVertex>&							m_MeshSet;
	std::vector<const SubmeshData*>						m_RenderObjectSubmesh;
	std::vector<int>									m_AniBoneIndices;
};