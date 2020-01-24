#pragma once
#include "DX12DrawSet.h"
#include "DX12MeshSet.h"
#include "DX12RenderClasses.h"
#include "DX12DefaultBuffer.h"
#include "DX12UploadBuffer.h"


class DX12DrawSetHeightField :public DX12DrawSet
{
	enum
	{
		OBJECT_CB = BASE_ROOT_PARAM_COUNT,
		ROOT_COUNT
	};

	enum
	{
		COMPUTE_HEIGHTS_SRV,
		COMPUTE_VERTICES_UAV,
		COMPUTE_INDICES_UAV,
		COMPUTE_FIELDINFO_CB,
		COMPUTE_ROOT_COUNT,
	};

	struct HeightFieldResultMesh
	{
		Microsoft::WRL::ComPtr<ID3D12Resource>	resultVertices;
		Microsoft::WRL::ComPtr<ID3D12Resource>	resultIndices;
		unsigned int							numVertices = 0;
		unsigned int							numIndices = 0;
		unsigned int							mapSize = 0;
	};

	struct FieldInfo
	{
		physx::PxVec3	Scale;
		unsigned int	MapSize;
		unsigned int	NumVertices;
	};
	
	const std::string heightFieldCreateCSName = "HFCreate";

public:
	DX12DrawSetHeightField(unsigned int numFrameResource,
		PSOController* psoCon,
		DX12TextureBuffer* textureBuffer,
		const std::vector<DXGI_FORMAT>& rtvFormats,
		DXGI_FORMAT dsvFormat, DX12MeshSet<float>& meshSet)
		: DX12DrawSet(numFrameResource, psoCon, textureBuffer, rtvFormats, dsvFormat)
		, m_MeshSet(meshSet)
	{

	}
	virtual ~DX12DrawSetHeightField() = default;

	virtual void	Init(ID3D12Device* device) override;
	virtual void	Draw(ID3D12GraphicsCommandList* cmd, const DX12PSOAttributeNames* custom = nullptr, const DX12_COMPUTE_CULLING_DESC* culling = nullptr) override;
	virtual void	ReserveRender(const RenderInfo& info) override;
	virtual void	UpdateFrameCountAndClearWork() override;
	void			ReComputeHeightField(const std::string& name, physx::PxVec3 scale, ID3D12Device* device, ID3D12GraphicsCommandList* cmd);

private:
	std::vector<std::unique_ptr<DX12UploadBuffer<DX12ObjectConstants>>>	m_MeshObjectCB;
	std::unordered_map<std::string, HeightFieldResultMesh>			m_ResultMesh;
	std::unique_ptr<DX12UploadBuffer<FieldInfo>>					m_FieldInfo;

	DX12MeshSet<float>&					m_MeshSet;
	std::vector<HeightFieldResultMesh*>	m_TargetMeshs;
};