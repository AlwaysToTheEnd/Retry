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
		std::vector<std::string> dataNames, 
		std::vector<T>& datas,
		D3D12_RESOURCE_STATES endState = D3D12_RESOURCE_STATE_GENERIC_READ);
	virtual ~cIndexManagementBuffer() = default;

public:
	UINT GetIndex(std::string& name) { return m_Indices.find(name)->second; }
	UINT GetDataNum() { return static_cast<UINT>(m_Indices.size()); }

private:
	std::unordered_map<std::string, UINT> m_Indices;
};

template<typename T>
inline cIndexManagementBuffer<T>::cIndexManagementBuffer(ID3D12Device* device, 
	ID3D12GraphicsCommandList* commandList, 
	std::vector<std::string> dataNames, 
	std::vector<T>& datas,
	D3D12_RESOURCE_STATES endState)
	:cDefaultBuffer<T>(device, commandList, datas, endState)
{
	for (size_t i = 0; i < datas.size(); i++)
	{
		m_Indices.insert({ dataNames[i],i });
	}
}
