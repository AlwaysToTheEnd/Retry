#pragma once
#include "BaseClass.h"
#include <d3d12.h>
#include <vector>
#include <wrl.h>


class DX12ShadowMap
{
public:
	DX12ShadowMap()
		: m_Index(m_CurrShadowMapNum)
	{
		m_CurrShadowMapNum++;
	}
	virtual ~DX12ShadowMap()=default;

	static void		ShadowMapDsvHeapCreate(ID3D12Device* device);
	static void		GetShadowMatrix(const physx::PxVec3& startSpot,const physx::PxVec3& dir, const physx::PxMat44& projection, physx::PxMat44& out);

	void						Resize(ID3D12Device* device, unsigned int width, unsigned int height);

	ID3D12Resource*				GetResource() { return m_Resource.Get(); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetDsv() const;
	physx::PxVec2				GetSize() const { return physx::PxVec2(m_Width, m_Height); }

private:
	void						CreateResource(ID3D12Device* device, unsigned int width, unsigned int height);

private:
	static Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>	m_DsvHeap;
	static unsigned int									m_DsvSize;
	static unsigned int									m_CurrShadowMapNum;

	const unsigned int									m_Index;
	Microsoft::WRL::ComPtr<ID3D12Resource>				m_Resource;
	unsigned int										m_Width;
	unsigned int										m_Height;
};