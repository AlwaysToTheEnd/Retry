#pragma once
#include "Vertex.h"
#include "DX12DrawSet.h"
#include "DX12RenderClasses.h"
#include "DX12DefaultBuffer.h"
#include "DX12UploadBuffer.h"

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
	DX12DrawSetSkinnedMesh(unsigned int numFrameResource)
		: DX12DrawSet(numFrameResource)
	{

	}
	virtual ~DX12DrawSetSkinnedMesh() = default;

	virtual void	Init(ID3D12Device * device, PSOController * psoCon,
								DXGI_FORMAT rtvFormat, DXGI_FORMAT dsvFormat,
								DX12TextureBuffer * textureBuffer,
								DX12IndexManagementBuffer<Material>* material, ID3D12Resource * mainPass) override;
	virtual void	Draw(ID3D12GraphicsCommandList * cmd, const PSOAttributeNames* custom = nullptr) override;
	virtual void	ReserveRender(const RenderInfo& info) override;
	virtual	void	UploadBuffersClear() override;

	void			UpdateAniBoneCB(const std::vector<AniBoneMat>& reservedData);
	bool			AddMesh(ID3D12Device * device, ID3D12GraphicsCommandList * commandList,
								const std::string & name, MeshObject & mesh,
								const std::vector<SkinnedVertex> & vertices, const std::vector<UINT> & indices);
	bool			AddMeshs(ID3D12Device * device, ID3D12GraphicsCommandList * commandList,
								const std::vector<std::string> & meshNames, const std::vector<MeshObject> & meshs,
								const std::vector<SkinnedVertex> & vertices, const std::vector<UINT> & indices);

	const std::unordered_map<std::string, MeshObject>* GetMeshs() const { return &m_Meshs; }

private:
	void CreateVertexIndexBuffer(ID3D12Device * device, ID3D12GraphicsCommandList * commandList,
		const std::vector<SkinnedVertex> & vertices, const std::vector<UINT> & indices);

private:
	std::vector<std::unique_ptr<DX12UploadBuffer<ObjectConstants>>>	m_MeshObjectCB;
	std::vector<std::unique_ptr<DX12UploadBuffer<AniBoneMat>>>		m_AniBoneCB;
	std::unordered_map<std::string, MeshObject>						m_Meshs;

	std::unique_ptr<DX12DefaultBuffer<SkinnedVertex>>	m_VertexBuffer;
	std::unique_ptr<DX12DefaultBuffer<UINT>>			m_IndexBuffer;

	std::vector<const SubmeshData*>						m_RenderObjectSubmesh;
	std::vector<int>									m_AniBoneIndices;
};