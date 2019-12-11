#pragma once
#include <wrl.h>
#include <d3d12.h>
#include "d3dx12.h"
#include "d3dUtil.h"

template<typename T>
class cDefaultBuffer
{
public:
	cDefaultBuffer() = delete;
	cDefaultBuffer(cDefaultBuffer& rhs) = delete;
	cDefaultBuffer& operator=(const cDefaultBuffer& rhs) = delete;
	cDefaultBuffer(ID3D12Device* device, 
		ID3D12GraphicsCommandList* commandList, 
		std::vector<T>& datas, 
		D3D12_RESOURCE_STATES endState = D3D12_RESOURCE_STATE_GENERIC_READ);
	virtual ~cDefaultBuffer() = default;

public:
	unsigned int GetBufferSize() { return m_BufferSize; }
	ID3D12Resource* GetBufferResource() { return m_Resource.Get(); }
	void ClearUploadBuffer() 
	{ 
		m_UploadBuffer = nullptr; 
	}

protected:
	unsigned int							m_BufferSize;
	Microsoft::WRL::ComPtr<ID3D12Resource>	m_Resource;
	Microsoft::WRL::ComPtr<ID3D12Resource>	m_UploadBuffer;
};

template<typename T>
inline cDefaultBuffer<T>::cDefaultBuffer(
	ID3D12Device* device,
	ID3D12GraphicsCommandList* commandList,
	std::vector<T>& datas,
	D3D12_RESOURCE_STATES endState)
{
	UINT elementByteSize = sizeof(T);
	BYTE* mappedData = nullptr;
	m_BufferSize = elementByteSize * datas.size();

	if (m_BufferSize)
	{
		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(m_BufferSize),
			D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
			IID_PPV_ARGS(m_Resource.GetAddressOf())));

		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(m_BufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
			IID_PPV_ARGS(m_UploadBuffer.GetAddressOf())));

		D3D12_SUBRESOURCE_DATA subResourceData = {};
		subResourceData.pData = datas.data();
		subResourceData.RowPitch = m_BufferSize;
		subResourceData.SlicePitch = subResourceData.RowPitch;

		UpdateSubresources<1>(commandList, m_Resource.Get(), m_UploadBuffer.Get(), 0, 0, 1, &subResourceData);
		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_Resource.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST, endState));
	}
}
