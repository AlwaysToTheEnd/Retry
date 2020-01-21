#pragma once
#include <d3d12.h>
#include <vector>
#include <wrl.h>


class DX12ShadowMap
{
public:
	DX12ShadowMap()=default;
	virtual ~DX12ShadowMap()=default;

	void Init(ID3D12Device* device, unsigned int numShadows);

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_ShadowMap

};