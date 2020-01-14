#pragma once
#include "cDefaultBuffer.h"

template<typename T>
class cIndexManagementBuffer: public cDefaultBuffer<T>
{
public:
	cIndexManagementBuffer() = delete;
	cIndexManagementBuffer(cIndexManagementBuffer& rhs) = delete;
	cIndexManagementBuffer& operator=(const cIndexManagementBuffer& rhs) = delete;
	cIndexManagementBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, 
							const std::vector<std::string>& dataNames,
							std::vector<T>& datas,
							D3D12_RESOURCE_STATES endState = D3D12_RESOURCE_STATE_GENERIC_READ);
	virtual ~cIndexManagementBuffer() = default;

	int										GetIndex(const std::string& name) const;
	UINT									GetDataNum() const { return static_cast<UINT>(m_Indices.size()); }

	Microsoft::WRL::ComPtr<ID3D12Resource>	IndexedAddData(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, UINT numData, const T* datas, const std::vector<std::string>& dataNames);
	bool									EditData(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, std::string name, const T* data);

private:
	std::unordered_map<std::string, UINT>	m_Indices;
	size_t									m_CurrIndex = 0;
};

template<typename T>
inline cIndexManagementBuffer<T>::cIndexManagementBuffer(ID3D12Device* device, 
	ID3D12GraphicsCommandList* commandList, 
	const std::vector<std::string>& dataNames,
	std::vector<T>& datas,
	D3D12_RESOURCE_STATES endState)
	:cDefaultBuffer<T>(device, commandList, datas, endState)
{
	for (size_t i = 0; i < datas.size(); i++)
	{
		m_Indices.insert({ dataNames[i], m_CurrIndex });
		m_CurrIndex++;
	}
}

template<typename T>
inline Microsoft::WRL::ComPtr<ID3D12Resource> cIndexManagementBuffer<T>::IndexedAddData(ID3D12Device* device,
	ID3D12GraphicsCommandList* commandList, UINT numData, const T* datas, 
	const std::vector<std::string>& dataNames)
{
	auto result = cDefaultBuffer<T>::AddData(device, commandList, numData, datas);

	for (size_t i = 0; i < numData; i++)
	{
		m_Indices.insert({ dataNames[i], m_CurrIndex });
		m_CurrIndex++;
	}

	return result;
}

template<typename T>
inline bool cIndexManagementBuffer<T>::EditData(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, std::string name, const T* data)
{
	auto iter = m_Indices.find(name);

	if (iter == m_Indices.end())
	{
		return false;
	}

	return cDefaultBuffer<T>::EditData(device, commandList, iter->second, data);
}

template<typename T>
inline int cIndexManagementBuffer<T>::GetIndex(const std::string& name) const
{
	int result = -1;
	auto iter = m_Indices.find(name);

	if (iter != m_Indices.end())
	{
		result = iter->second;
	}

	return result;
}
