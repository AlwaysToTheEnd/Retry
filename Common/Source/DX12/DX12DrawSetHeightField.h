#pragma once
#include "DX12DrawSet.h"
#include "DX12MeshSet.h"
#include "DX12RenderClasses.h"
#include "DX12DefaultBuffer.h"
#include "DX12UploadBuffer.h"


class DX12DrawSetHeightField :public DX12DrawSet
{
	/*struct DX12HeightFieldMesh
	{
		DX12HeightFieldMesh(ID3D12Device* device, unsigned int numHeights)
			: scale(1, 1, 1)
			, offsetPos(0, 0, 0)
		{
			heights = std::make_unique<DX12UploadBuffer<float>>(device, numHeights, false);

			D3D12_HEAP_PROPERTIES vhp = {};
			vhp.Type = D3D12_HEAP_TYPE_DEFAULT;
			vhp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE;
			vhp.MemoryPoolPreference = D3D12_MEMORY_POOL_L1;
			vhp.CreationNodeMask = 1;
			vhp.VisibleNodeMask = 1;

			D3D12_RESOURCE_DESC vhrd = {};
			vhrd.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			vhrd.Width = numHeights * sizeof(Vertex);
			vhrd.Height = 1;
			vhrd.DepthOrArraySize = 1;
			vhrd.MipLevels = 1;
			vhrd.Format = DXGI_FORMAT_UNKNOWN;
			vhrd.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			vhrd.SampleDesc.Count = 1;
			vhrd.SampleDesc.Quality = 0;

			ThrowIfFailed(device->CreateCommittedResource(&vhp, D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS, &vhrd, D3D12_RESOURCE_STATE_COMMON,
				nullptr, IID_PPV_ARGS(vertices.GetAddressOf())));

			vhrd.Width = numHeights * sizeof(UINT);

			ThrowIfFailed(device->CreateCommittedResource(&vhp, D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS, &vhrd, D3D12_RESOURCE_STATE_COMMON,
				nullptr, IID_PPV_ARGS(indices.GetAddressOf())));
		}

		std::unique_ptr<DX12UploadBuffer<float>>	heights;
		Microsoft::WRL::ComPtr<ID3D12Resource>		vertices;
		Microsoft::WRL::ComPtr<ID3D12Resource>		indices;
		physx::PxVec3								scale;
		physx::PxVec3								offsetPos;
	};*/

	enum
	{
		OBJECT_CB = BASE_ROOT_PARAM_COUNT,
		HEIGHT_SRV,
		ROOT_COUNT
	};

	struct HFObjectData
	{
		physx::PxMat44  world;
		physx::PxVec3	scale;
		int				diffuseMapIndex;
		int				normalMapIndex;
		int				materialIndex;
		unsigned int	mapSize;
		unsigned int	numVertices;
	};
	
public:
	DX12DrawSetHeightField(unsigned int numFrameResource,
		PSOController* psoCon,
		DX12TextureBuffer* textureBuffer,
		DX12IndexManagementBuffer<Material>* material,
		ID3D12Resource* mainPass, const std::vector<DXGI_FORMAT>& rtvFormats,
		DXGI_FORMAT dsvFormat, DX12MeshSet<float>& meshSet)
		: DX12DrawSet(numFrameResource, psoCon, textureBuffer, material, mainPass, rtvFormats, dsvFormat)
		, m_MeshSet(meshSet)
	{

	}
	virtual ~DX12DrawSetHeightField() = default;

	virtual void	Init(ID3D12Device* device) override;
	virtual void	Draw(ID3D12GraphicsCommandList* cmd, const PSOAttributeNames* custom = nullptr) override;
	virtual void	ReserveRender(const RenderInfo& info) override;

private:
	std::vector<std::unique_ptr<DX12UploadBuffer<HFObjectData>>>	m_MeshObjectCB;

	DX12MeshSet<float>&					m_MeshSet;
	std::vector<const SubmeshData*>		m_RenderObjectSubmesh;
};