#pragma once
#include <wrl.h>
#include <d3d12.h>
#include "d3dx12.h"
#include "d3dUtil.h"

template<typename T>
class DX12DefaultBuffer
{
public:
	DX12DefaultBuffer() = delete;
	DX12DefaultBuffer(DX12DefaultBuffer& rhs) = delete;
	DX12DefaultBuffer& operator=(const DX12DefaultBuffer& rhs) = delete;
	DX12DefaultBuffer(ID3D12Device* device,
		ID3D12GraphicsCommandList* commandList,
		const std::vector<T>& datas,
		D3D12_RESOURCE_STATES endState = D3D12_RESOURCE_STATE_GENERIC_READ);
	virtual ~DX12DefaultBuffer() = default;

	void				ClearUploadBuffer() { m_UploadBuffer = nullptr; }

	UINT				GetBufferSize() { return m_BufferSize; }
	UINT				GetNumDatas() { return (m_BufferSize - m_Redundancy) / sizeof(T); }
	ID3D12Resource*		GetBufferResource() { return m_Resource.Get(); }

	bool									AddData(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, UINT numData, const T* datas);
	bool									EditData(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, UINT index, const T* data);
	bool									EditDatas(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, UINT objectNumOffset, UINT dataNum, const T* datas);

protected:
	UINT									m_BufferSize;
	UINT									m_Redundancy;
	Microsoft::WRL::ComPtr<ID3D12Resource>	m_Resource;
	Microsoft::WRL::ComPtr<ID3D12Resource>	m_UploadBuffer;
};

template<typename T>
inline DX12DefaultBuffer<T>::DX12DefaultBuffer(
	ID3D12Device* device,
	ID3D12GraphicsCommandList* commandList,
	const std::vector<T>& datas,
	D3D12_RESOURCE_STATES endState)
{
	UINT elementByteSize = sizeof(T);
	BYTE* mappedData = nullptr;
	m_BufferSize = elementByteSize * datas.size();
	m_Redundancy = 0;

	if (m_BufferSize == 0)
	{
		m_BufferSize = 256;
		m_Redundancy = 256;

		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(m_BufferSize),
			endState, nullptr,
			IID_PPV_ARGS(m_Resource.GetAddressOf())));
	}
	else
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

template<typename T>
inline bool DX12DefaultBuffer<T>::AddData(ID3D12Device* device,
	ID3D12GraphicsCommandList* commandList, UINT numData, const T* datas)
{
	Microsoft::WRL::ComPtr<ID3D12Resource> result = nullptr;
	UINT elementByteSize = sizeof(T);
	BYTE* mappedData = nullptr;
	UINT addDataByteSize = elementByteSize * numData;
	UINT usingByte = m_BufferSize - m_Redundancy;

	if (m_Redundancy < addDataByteSize)
	{
		m_BufferSize = (m_BufferSize * 2) > (m_BufferSize + addDataByteSize) ? (m_BufferSize * 2) : (m_BufferSize + addDataByteSize);
		m_Redundancy = m_BufferSize - usingByte;

		Microsoft::WRL::ComPtr<ID3D12Resource> newResource;
		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(m_BufferSize),
			D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
			IID_PPV_ARGS(newResource.GetAddressOf())));

		commandList->CopyBufferRegion(newResource.Get(), 0, m_Resource.Get(), 0, usingByte);

		result = m_Resource;
		m_Resource = nullptr;
		m_Resource = newResource;
	}
	else
	{
		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_Resource.Get(),
			D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST));
	}

	assert(m_UploadBuffer == nullptr);

	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(addDataByteSize),
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(m_UploadBuffer.GetAddressOf())));

	BYTE* data = nullptr;
	m_UploadBuffer->Map(0, &CD3DX12_RANGE(0, 0), reinterpret_cast<void**>(&data));
	std::memcpy(data, datas, addDataByteSize);
	m_UploadBuffer->Unmap(0, nullptr);

	commandList->CopyBufferRegion(m_Resource.Get(), usingByte, m_UploadBuffer.Get(), 0, addDataByteSize);
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_Resource.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

	m_Redundancy -= addDataByteSize;

	if (result)
	{
		result->Release();
	}

	return true;
}

template<typename T>
inline bool DX12DefaultBuffer<T>::EditData(ID3D12Device* device,
	ID3D12GraphicsCommandList* commandList, UINT index, const T* data)
{
	const int byteSize = sizeof(T);

	if ((m_BufferSize - m_Redundancy) < ((index+1) * byteSize))
	{
		return false;
	}

	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_Resource.Get(),
		D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST));

	assert(m_UploadBuffer == nullptr);

	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(byteSize),
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(m_UploadBuffer.GetAddressOf())));

	BYTE* bufferData = nullptr;
	m_UploadBuffer->Map(0, &CD3DX12_RANGE(0, 0), reinterpret_cast<void**>(&bufferData));
	std::memcpy(bufferData, data, byteSize);
	m_UploadBuffer->Unmap(0, nullptr);

	commandList->CopyBufferRegion(m_Resource.Get(), index* byteSize, m_UploadBuffer.Get(), 0, byteSize);
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_Resource.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

	return true;
}

template<typename T>
inline bool DX12DefaultBuffer<T>::EditDatas(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, UINT objectNumOffset, UINT dataNum, const T* datas)
{
	const int byteSize = sizeof(T) * dataNum;

	if ((m_BufferSize - m_Redundancy) < ((dataNum + objectNumOffset) * sizeof(T)))
	{
		return false;
	}

	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_Resource.Get(),
		D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST));

	assert(m_UploadBuffer == nullptr);

	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(byteSize),
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(m_UploadBuffer.GetAddressOf())));

	BYTE* bufferData = nullptr;
	m_UploadBuffer->Map(0, &CD3DX12_RANGE(0, 0), reinterpret_cast<void**>(&bufferData));
	std::memcpy(bufferData, datas, byteSize);
	m_UploadBuffer->Unmap(0, nullptr);

	commandList->CopyBufferRegion(m_Resource.Get(), objectNumOffset* sizeof(T), m_UploadBuffer.Get(), 0, byteSize);
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_Resource.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

	return true;
}
