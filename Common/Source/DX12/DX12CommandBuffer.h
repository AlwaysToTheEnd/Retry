#pragma once
#include <d3d12.h>
#include <d3dx12.h>
#include <wrl.h>
#include "d3dUtil.h"

template <typename T>
class DX12CommandBuffer
{
public:
	DX12CommandBuffer(ID3D12Device* device, unsigned int numObject, D3D12_CPU_DESCRIPTOR_HANDLE uav);
	~DX12CommandBuffer()= default;

	ID3D12Resource* Resource() { return m_Resource.Get(); }
	void			DataMappingAndCountClear(const std::vector<T>& data);
	unsigned int	GetCommandBufferCounterOffset() { return AlignForUavCounter(m_NumObject * sizeof(T)); }

private:
	static inline UINT AlignForUavCounter(UINT bufferSize)
	{
		const UINT alignment = D3D12_UAV_COUNTER_PLACEMENT_ALIGNMENT;
		return (bufferSize + (alignment - 1)) & ~(alignment - 1);
	}

private:
	Microsoft::WRL::ComPtr<ID3D12Resource>	m_Resource;
	unsigned int							m_NumObject;
	unsigned int							m_CounterOffset;
};


template<typename T>
inline DX12CommandBuffer<T>::DX12CommandBuffer(ID3D12Device* device, unsigned int numObject, D3D12_CPU_DESCRIPTOR_HANDLE uav)
{
	m_NumObject = numObject;
	m_CounterOffset = AlignForUavCounter(m_NumObject * sizeof(T));

	D3D12_RESOURCE_DESC commandBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(m_CounterOffset + sizeof(UINT), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&commandBufferDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(m_Resource.GetAddressOf())));

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = m_NumObject;
	uavDesc.Buffer.StructureByteStride = sizeof(T);
	uavDesc.Buffer.CounterOffsetInBytes = m_CounterOffset;
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

	device->CreateUnorderedAccessView(m_Resource.Get(), m_Resource.Get(), &uavDesc, uav);
}

template<typename T>
inline void DX12CommandBuffer<T>::DataMappingAndCountClear(const std::vector<T>& data)
{
	T* bufferPtr = nullptr;
	UINT dataSize = sizeof(T);
	UINT countZero = 0;
	D3D12_RANGE range = { 0, 0 };
	m_Resource->Map(0, &range, reinterpret_cast<void**>(&bufferPtr));
	std::memcpy(bufferPtr, data.data(), data.size()* dataSize);
	std::memcpy(bufferPtr + (dataSize * m_NumObject), &countZero, sizeof(UINT));
	m_Resource->Unmap(0, nullptr);
}
