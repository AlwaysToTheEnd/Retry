#pragma once
#include "Vertex.h"
#include "DX12RenderClasses.h"
#include "DX12CommandBuffer.h"
#include "DX12DrawSet.h"
#include "DX12MeshSet.h"

struct DX12NormalMeshIndirectCommand
{
	D3D12_GPU_VIRTUAL_ADDRESS		cbv;
	D3D12_DRAW_INDEXED_ARGUMENTS	draw;
};

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
		DX12IndexManagementBuffer<Material>* material,
		const std::vector<DXGI_FORMAT>& rtvFormats,
		DXGI_FORMAT dsvFormat, DX12MeshSet<Vertex>& meshSet)
		: DX12DrawSet(numFrameResource, psoCon, textureBuffer, material, rtvFormats, dsvFormat)
		, m_MeshSet(meshSet)
	{

	}
	virtual ~DX12DrawSetNormalMesh() = default;

	virtual void	Init(ID3D12Device* device) override;
	virtual void	Draw(ID3D12GraphicsCommandList * cmd, const DX12PSOAttributeNames* custom = nullptr) override;
	virtual void	ReserveRender(const RenderInfo& info) override;
	virtual void	UpdateFrameCountAndClearWork() override;

private:
	void ResizeCurrFrameCB();

private:
	std::vector<std::unique_ptr<DX12UploadBuffer<DX12ObjectConstants>>>				m_MeshObjectCB;
	std::vector<std::unique_ptr<DX12CommandBuffer<DX12NormalMeshIndirectCommand>>>	m_Commands;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>									m_CommandUAVHeap;

	DX12MeshSet<Vertex>&						m_MeshSet;
	std::vector<const SubmeshData*>				m_RenderObjectSubmesh;
};