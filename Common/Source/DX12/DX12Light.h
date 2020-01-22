#pragma once
#include "DX12ShadowMap.h"
#include "DX12DrawSet.h"
#include "DX12RenderClasses.h"
#include <memory>
#include <initializer_list>

enum class DX12_LIGHT_TYPE
{
	DX12_LIGHT_TYPE_DIRECTIONAL,
	DX12_LIGHT_TYPE_POINT,
	DX12_LIGHT_TYPE_SPOT,
};

class DX12Light
{
public:

	void SetColor(const physx::PxVec3& color);
	void InitDirectionalLight(const physx::PxVec3& dir);
	void InitPointLight(const physx::PxVec3& pos, float fallOffStart, float fallOffEnd);
	void InitSpotLight(const physx::PxVec3& pos, const physx::PxVec3& dir, float fallOffStart, float fallOffEnd);
	
	void SetShadowMap(ID3D12Device* device, unsigned int width, unsigned int height);
	void ShadowMapUpdate(ID3D12GraphicsCommandList* cmd, std::initializer_list<DX12DrawSet*> targets);

private:
	DX12_LIGHT_VALUE								m_Value;
	std::unique_ptr<DX12ShadowMap>	m_ShadowMap;
};