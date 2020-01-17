#pragma once
#include "Vertex.h"
#include "DX12RenderClasses.h"
#include "DX12DrawSet.h"
#include "DX12MeshSet.h"

class DX12DrawSetSkinnedMesh : public DX12DrawSet
{
	enum
	{
		PASS_CB,
		TEXTURE_TABLE,
		MATERIAL_SRV,
		OBJECT_CB,
		ANIBONE_CB,
		ROOT_COUNT
	};

public:
	DX12DrawSetSkinnedMesh(unsigned int numFrameResource, DX12MeshSet<SkinnedVertex>& meshSet)
		: DX12DrawSet(numFrameResource)
		, m_MeshSet(meshSet)
	{

	}
	virtual ~DX12DrawSetSkinnedMesh() = default;

	virtual void	Init(ID3D12Device * device, PSOController * psoCon,
								DXGI_FORMAT rtvFormat, DXGI_FORMAT dsvFormat,
								DX12TextureBuffer * textureBuffer,
								DX12IndexManagementBuffer<Material>* material, ID3D12Resource * mainPass) override;
	virtual void	Draw(ID3D12GraphicsCommandList * cmd, const PSOAttributeNames* custom = nullptr) override;
	virtual void	ReserveRender(const RenderInfo& info) override;

	void			UpdateAniBoneCB(const std::vector<AniBoneMat>& reservedData);


private:
	std::vector<std::unique_ptr<DX12UploadBuffer<ObjectConstants>>>	m_MeshObjectCB;
	std::vector<std::unique_ptr<DX12UploadBuffer<AniBoneMat>>>		m_AniBoneCB;

	DX12MeshSet<SkinnedVertex>&							m_MeshSet;
	std::vector<const SubmeshData*>						m_RenderObjectSubmesh;
	std::vector<int>									m_AniBoneIndices;
};