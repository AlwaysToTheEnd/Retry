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
	unsigned int	GetCommandBufferCounterOffset() { return m_NumObject * sizeof(T); }

private:
	Microsoft::WRL::ComPtr<ID3D12Resource>	m_Resource;
	unsigned int							m_NumObject;
};


template<typename T>
inline DX12CommandBuffer<T>::DX12CommandBuffer(ID3D12Device* device, unsigned int numObject, D3D12_CPU_DESCRIPTOR_HANDLE uav)
{
	m_NumObject = numObject;
	const unsigned int commandBufferCounterOffset = m_NumObject * sizeof(T);

	D3D12_RESOURCE_DESC commandBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(commandBufferCounterOffset + sizeof(UINT), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
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
	uavDesc.Buffer.CounterOffsetInBytes = commandBufferCounterOffset;
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

	ThrowIfFailed(device->CreateUnorderedAccessView(m_Resource.Get(), m_Resource.Get(), &uavDesc, uav));
}
