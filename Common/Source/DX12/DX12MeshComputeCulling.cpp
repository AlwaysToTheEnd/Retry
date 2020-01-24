#include "DX12MeshComputeCulling.h"
#include <d3dx12.h>

Microsoft::WRL::ComPtr<ID3D12Resource> DX12MeshComputeCulling::m_Zero = nullptr;
unsigned int DX12MeshComputeCulling::m_InstanceCount = 0;
PSOController* DX12MeshComputeCulling::m_PsoCon = nullptr;
unsigned int DX12MeshComputeCulling::m_UavSrvSize = 0;
const DX12_COMPUTE_CULLING_FRUSTUM* DX12MeshComputeCulling::m_BaseFrustum = nullptr;

DX12MeshComputeCulling::DX12MeshComputeCulling()
	: m_CurrCDBufferIndex(0)
	, m_NumObject(0)
	, m_ObjectStride(0)
	, m_CounterOffset(0)
{
	m_InstanceCount++;
}

DX12MeshComputeCulling::~DX12MeshComputeCulling()
{
	m_InstanceCount--;
	
	if (m_InstanceCount <= 0)
	{
		m_Zero = nullptr;
	}
}

void DX12MeshComputeCulling::BaseSetting(ID3D12Device* device, PSOController* psocon, const DX12_COMPUTE_CULLING_FRUSTUM* basFrustum)
{
	m_PsoCon = psocon;
	m_BaseFrustum = basFrustum;
	m_UavSrvSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_DESCRIPTOR_RANGE tableRange[2] = {};
	tableRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	tableRange[0].NumDescriptors = 2;
	tableRange[0].BaseShaderRegister = 0;
	tableRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	tableRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	tableRange[1].NumDescriptors = 1;
	tableRange[1].BaseShaderRegister = 0;
	tableRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	CD3DX12_ROOT_PARAMETER computeRootParam[COMPUTE_ROOT_COUNT];
	computeRootParam[COMPUTE_RESOURCE_TABLE].InitAsDescriptorTable(_countof(tableRange), tableRange);
	computeRootParam[COMPUTE_CULLINGINFO_CB].InitAsConstantBufferView(0);

	CD3DX12_ROOT_SIGNATURE_DESC computeRootDesc;
	computeRootDesc.Init(COMPUTE_ROOT_COUNT, computeRootParam);
	m_PsoCon->AddRootSignature("cullingCompute", computeRootDesc);

	D3D_SHADER_MACRO cullingMacros[] =
	{
		"SKINNED", NULL,
		NULL, NULL
	};
	m_PsoCon->AddShader("skinnedCulling", DX12_SHADER_COMPUTE, L"../Common/MainShaders/CullingCompute.hlsl", cullingMacros, "CS");
	m_PsoCon->AddShader("normalCulling", DX12_SHADER_COMPUTE, L"../Common/MainShaders/CullingCompute.hlsl", nullptr, "CS");

	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(unsigned int)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_Zero.GetAddressOf())));

	unsigned int* ZeroData = nullptr;
	m_Zero->Map(0, &CD3DX12_RANGE(0, 0), reinterpret_cast<void**>(&ZeroData));
	ZeroMemory(ZeroData, sizeof(unsigned int));
	m_Zero->Unmap(0, nullptr);
}

void DX12MeshComputeCulling::Init(ID3D12Device* device, PSOController* psocon, FrameObjectCBs& obCB, FrameUploadSRVs& srvs, unsigned int objectNum, unsigned int objectStride)
{
	m_NumObject = objectNum;
	m_ObjectStride = objectStride;
	m_CounterOffset = AlignForUavCounter(m_NumObject * m_ObjectStride);
	CreateResourceAndViewHeap(device, obCB, srvs);
}

ID3D12Resource* DX12MeshComputeCulling::Compute(ID3D12GraphicsCommandList* cmd, unsigned int numDatas, unsigned int frame, const std::string& csName, const DX12_COMPUTE_CULLING_DESC* culling)
{
	cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_Commands[frame].Get(),
		D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT, D3D12_RESOURCE_STATE_COPY_DEST));

	DX12_COMPUTE_CULLING_DESC cull = {};
	if (culling)
	{
		memcpy(&cull, culling, sizeof(DX12_COMPUTE_CULLING_DESC));
		cull.numObjects = numDatas;
	}
	else
	{
		cull.type = DX12_COMPUTE_CULLING_TYPE_FRUSTUM;
		cull.numObjects = numDatas;
		cull.frustum = *m_BaseFrustum;
	}

	m_CullingDescBuffer->CopyData(m_CurrCDBufferIndex, &cull);

	cmd->CopyBufferRegion(m_Commands[frame].Get(), m_CounterOffset, m_Zero.Get(), 0, sizeof(unsigned int));

	cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_Commands[frame].Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

	m_PsoCon->SetPSOToCommnadList(cmd, "cullingCompute", csName);
	auto heapPtr = m_CommandSRVUAVHeap->GetGPUDescriptorHandleForHeapStart();
	heapPtr.ptr += (m_UavSrvSize * 3) * frame;

	auto cbPtr = m_CullingDescBuffer->Resource()->GetGPUVirtualAddress();
	auto cbElementSize = m_CullingDescBuffer->GetElementByteSize();

	ID3D12DescriptorHeap* heaps[] = { m_CommandSRVUAVHeap.Get() };
	cmd->SetDescriptorHeaps(1, heaps);
	cmd->SetComputeRootDescriptorTable(COMPUTE_RESOURCE_TABLE, heapPtr);
	cmd->SetComputeRootConstantBufferView(COMPUTE_CULLINGINFO_CB, cbPtr + (cbElementSize * m_CurrCDBufferIndex));
	cmd->Dispatch(numDatas, 1, 1);

	cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_Commands[frame].Get(),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT));

	m_CurrCDBufferIndex = (m_CurrCDBufferIndex + 1) % (MaxCullingNumPerFrame * m_Commands.size());

	return m_Commands[frame].Get();
}

void DX12MeshComputeCulling::CreateResourceAndViewHeap(ID3D12Device* device, FrameObjectCBs& obCB, FrameUploadSRVs& srvs)
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
	heapDesc.NumDescriptors = obCB.size() * 3;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.NodeMask = 1;

	ThrowIfFailed(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_CommandSRVUAVHeap.GetAddressOf())));
	auto heapHandle = m_CommandSRVUAVHeap->GetCPUDescriptorHandleForHeapStart();

	D3D12_SHADER_RESOURCE_VIEW_DESC obCBDesc = {};
	obCBDesc.Format = DXGI_FORMAT_UNKNOWN;
	obCBDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	obCBDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	obCBDesc.Buffer.NumElements = obCB[0]->GetNumElement();
	obCBDesc.Buffer.StructureByteStride = obCB[0]->GetElementByteSize();
	obCBDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.NumElements = m_NumObject;
	srvDesc.Buffer.StructureByteStride = m_ObjectStride;
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = m_NumObject;
	uavDesc.Buffer.StructureByteStride = m_ObjectStride;
	uavDesc.Buffer.CounterOffsetInBytes = m_CounterOffset;
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

	m_Commands.resize(obCB.size());

	for (int i = 0; i < obCB.size(); i++)
	{
		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(m_CounterOffset + sizeof(unsigned int), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
			D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT,
			nullptr,
			IID_PPV_ARGS(m_Commands[i].GetAddressOf())));

		device->CreateShaderResourceView(obCB[i]->Resource(), &obCBDesc, heapHandle);
		heapHandle.ptr += m_UavSrvSize;
		device->CreateShaderResourceView(srvs[i], &srvDesc, heapHandle);
		heapHandle.ptr += m_UavSrvSize;
		device->CreateUnorderedAccessView(m_Commands[i].Get(), m_Commands[i].Get(), &uavDesc, heapHandle);
		heapHandle.ptr += m_UavSrvSize;
	}

	m_CullingDescBuffer = std::make_unique<CullingDescBuffer>(device, MaxCullingNumPerFrame * obCB.size(), true);
}
