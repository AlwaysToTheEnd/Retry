#include "DX12Light.h"
#include <DirectXMath.h>
using namespace DirectX;

void DX12Light::SetColor(const physx::PxVec3& color)
{
	m_Value.lightColor = color;
}

void DX12Light::InitDirectionalLight(const physx::PxVec3& dir)
{
	m_Value.type = static_cast<unsigned int>(DX12_LIGHT_TYPE::DX12_LIGHT_TYPE_DIRECTIONAL);
	m_Value.direction = dir;
}

void DX12Light::InitPointLight(const physx::PxVec3& pos, float fallOffStart, float fallOffEnd)
{
	m_Value.type = static_cast<unsigned int>(DX12_LIGHT_TYPE::DX12_LIGHT_TYPE_POINT);

	m_Value.position = pos;
	m_Value.falloffStart = fallOffStart;
	m_Value.falloffEnd = fallOffEnd;
}

void DX12Light::InitSpotLight(const physx::PxVec3& pos, const physx::PxVec3& dir, float fallOffStart, float fallOffEnd)
{
	m_Value.type = static_cast<unsigned int>(DX12_LIGHT_TYPE::DX12_LIGHT_TYPE_SPOT);
	m_Value.position = pos;
	m_Value.direction = dir;
}

void DX12Light::SetShadowMap(ID3D12Device* device, unsigned int width, unsigned int height)
{
	if (m_ShadowMap == nullptr)
	{
		m_ShadowMap = std::make_unique<DX12ShadowMap>();
	}

	m_ShadowMap->Resize(device, width, height);
}

void DX12Light::ShadowMapUpdate(ID3D12GraphicsCommandList* cmd, std::initializer_list<DX12DrawSet*> targets)
{
	if (m_ShadowMap.get() == nullptr) return;

	cmd->OMSetRenderTargets(0, nullptr, true, &m_ShadowMap->GetDsv());
	XMMATRIX shadowProj = XMMatrixIdentity();

	switch (DX12_LIGHT_TYPE(m_Value.type))
	{
	case DX12_LIGHT_TYPE::DX12_LIGHT_TYPE_DIRECTIONAL:
		static const float radius = 500.0f;
		shadowProj = XMMatrixOrthographicOffCenterLH(-radius, radius, -radius, radius, -radius, radius);
		break;
	case DX12_LIGHT_TYPE::DX12_LIGHT_TYPE_POINT:
		break;
	case DX12_LIGHT_TYPE::DX12_LIGHT_TYPE_SPOT:
		break;
	default:
		break;
	}

	DX12PSOAttributeNames psocon;
	psocon.dsvFormat = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
}
