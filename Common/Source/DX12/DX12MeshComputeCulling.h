#pragma once
#include <memory>
#include <d3d12.h>
#include <wrl.h>
#include <vector>
#include "PSOController.h"
#include "DX12UploadBuffer.h"

enum DX12_COMPUTE_CULLING_ROOT
{
	COMPUTE_RESOURCE_TABLE,
	COMPUTE_OBJECTNUM_CONST,
	COMPUTE_ROOT_COUNT,
};

typedef std::vector<std::unique_ptr<DX12UploadBuffer<DX12ObjectConstants>>> FrameObjectCBs;

class DX12MeshComputeCulling
{
public:
	DX12MeshComputeCulling();
	virtual ~DX12MeshComputeCulling();

	static void			BaseSetting(ID3D12Device* device, PSOController* psocon);
	void				Init(ID3D12Device* device, PSOController* psocon, FrameObjectCBs& obCB, unsigned int objectNum, unsigned int objectStride);
	
	ID3D12Resource*		Compute(ID3D12GraphicsCommandList* cmd, unsigned int numDatas, ID3D12Resource* uploadBuffer, unsigned int frame, const std::string& csName);
	unsigned int		GetCounterOffset() const { return m_CounterOffset; }

private:
	static inline unsigned int AlignForUavCounter(unsigned int bufferSize)
	{
		const unsigned int alignment = D3D12_UAV_COUNTER_PLACEMENT_ALIGNMENT;
		return (bufferSize + (alignment - 1)) & ~(alignment - 1);
	}

	void CreateResourceAndViewHeap(ID3D12Device* device, FrameObjectCBs& obCB);

private:
	static Microsoft::WRL::ComPtr<ID3D12Resource>		m_Zero;
	static unsigned int									m_InstanceCount;
	static PSOController*								m_PsoCon;
	static unsigned int									m_UavSrvSize;

	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>>	m_Commands;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>		m_CommandSRVUAVHeap;
	unsigned int										m_NumObject;
	unsigned int										m_ObjectStride;
	unsigned int										m_CounterOffset;
};
