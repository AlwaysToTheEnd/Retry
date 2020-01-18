#pragma once
#include "Vertex.h"
#include "DX12RenderClasses.h"
#include "DX12DrawSet.h"
#include "DX12MeshSet.h"

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
		ID3D12Resource* mainPass, const std::vector<DXGI_FORMAT>& rtvFormats,
		DXGI_FORMAT dsvFormat, DX12MeshSet<Vertex>& meshSet)
		: DX12DrawSet(numFrameResource, psoCon, textureBuffer, material, mainPass, rtvFormats, dsvFormat)
		, m_MeshSet(meshSet)
	{

	}
	virtual ~DX12DrawSetNormalMesh() = default;

	virtual void	Init(ID3D12Device* device) override;
	virtual void	Draw(ID3D12GraphicsCommandList * cmd, const PSOAttributeNames* custom = nullptr) override;
	virtual void	ReserveRender(const RenderInfo& info) override;

private:
	void ResizeCurrFrameCB();

private:
	std::vector<std::unique_ptr<DX12UploadBuffer<ObjectConstants>>>	m_MeshObjectCB;

	DX12MeshSet<Vertex>&						m_MeshSet;
	std::vector<const SubmeshData*>				m_RenderObjectSubmesh;
};