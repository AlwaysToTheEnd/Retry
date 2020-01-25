#pragma once
#include <memory>
#include <d3d12.h>
#include <wrl.h>
#include <vector>
#include "PSOController.h"
#include "DX12UploadBuffer.h"

typedef std::vector<std::unique_ptr<DX12UploadBuffer<DX12ObjectConstants>>> FrameObjectCBs;
typedef std::vector<ID3D12Resource*> FrameUploadSRVs;

enum DX12_COMPUTE_CULLING_TYPE
{
	DX12_COMPUTE_CULLING_TYPE_FRUSTUM,
	DX12_COMPUTE_CULLING_TYPE_SPHERE,
	DX12_COMPUTE_CULLING_TYPE_BOX,
	DX12_COMPUTE_CULLING_TYPE_CON,
};

#pragma pack(push, 4)
struct DX12_COMPUTE_CULLING_FRUSTUM
{
	physx::PxVec4	rightNormal;
	physx::PxVec4	leftNormal;
	physx::PxVec4	upNormal;
	physx::PxVec4	downNormal;
	physx::PxMat44	viewMat;
	physx::PxVec2	near_Far;
};

struct DX12_COMPUTE_CULLING_SPHERE
{
	physx::PxVec4	pos_Radian;
};

struct DX12_COMPUTE_CULLING_BOX
{
	physx::PxVec4	pos;
	physx::PxVec4	halfSize;
};

struct DX12_COMPUTE_CULLING_CON
{
	physx::PxVec4	pos_Length;
	physx::PxVec4	dir_cos;
};

struct DX12_COMPUTE_CULLING_DESC
{
	union
	{
		DX12_COMPUTE_CULLING_FRUSTUM	frustum;
		DX12_COMPUTE_CULLING_SPHERE		sphere;
		DX12_COMPUTE_CULLING_BOX		box;
		DX12_COMPUTE_CULLING_CON		con;
	};

	DX12_COMPUTE_CULLING_TYPE	type;
	unsigned int				numObjects;
};
#pragma pack(pop)

class DX12MeshComputeCulling
{
	typedef DX12UploadBuffer<DX12_COMPUTE_CULLING_DESC> CullingDescBuffer;
	static const unsigned int MaxCullingNumPerFrame = 20;

	enum
	{
		COMPUTE_RESOURCE_TABLE,
		COMPUTE_CULLINGINFO_CB,
		COMPUTE_ROOT_COUNT,
	};

public:
	DX12MeshComputeCulling();
	virtual ~DX12MeshComputeCulling();

	static void			BaseSetting(ID3D12Device* device, PSOController* psocon, const DX12_COMPUTE_CULLING_FRUSTUM* basFrustum);
	void				Init(ID3D12Device* device, PSOController* psocon, FrameObjectCBs& obCB, FrameUploadSRVs& srvs, unsigned int objectNum, unsigned int objectStride);
	
	ID3D12Resource*		RenderCompute(ID3D12GraphicsCommandList* cmd, unsigned int numDatas, unsigned int frame, const std::string& csName, const DX12_COMPUTE_CULLING_DESC* culling);
	unsigned int		GetCounterOffset() const { return m_CounterOffset; }

private:
	static inline unsigned int AlignForUavCounter(unsigned int bufferSize)
	{
		const unsigned int alignment = D3D12_UAV_COUNTER_PLACEMENT_ALIGNMENT;
		return (bufferSize + (alignment - 1)) & ~(alignment - 1);
	}

	void CreateResourceAndViewHeap(ID3D12Device* device, FrameObjectCBs& obCB, FrameUploadSRVs& srvs);

private:
	static Microsoft::WRL::ComPtr<ID3D12Resource>		m_Zero;
	static unsigned int									m_InstanceCount;
	static PSOController*								m_PsoCon;
	static const DX12_COMPUTE_CULLING_FRUSTUM*			m_BaseFrustum;
	static unsigned int									m_UavSrvSize;

	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>>	m_Commands;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>		m_CommandSRVUAVHeap;
	std::unique_ptr<CullingDescBuffer>					m_CullingDescBuffer;
	unsigned int										m_CurrCDBufferIndex;
	unsigned int										m_NumObject;
	unsigned int										m_ObjectStride;
	unsigned int										m_CounterOffset;
};
