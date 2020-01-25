#include "DX12Light.h"
#include <DirectXMath.h>
using namespace DirectX;

void DX12Light::SetColor(const physx::PxVec3& color)
{
	m_Value.lightColor = color;
}

void DX12Light::InitDirectionalLight(const physx::PxVec3& dir)
{
	m_Value.type = LIGHT_TYPE_DIRECTIONAL;
	m_Value.direction = dir;
}

void DX12Light::InitPointLight(const physx::PxVec3& pos, float fallOffStart, float fallOffEnd)
{
	m_Value.type = LIGHT_TYPE_POINT;

	m_Value.position = pos;
	m_Value.falloffStart = fallOffStart;
	m_Value.falloffEnd = fallOffEnd;
}

void DX12Light::InitSpotLight(const physx::PxVec3& pos, const physx::PxVec3& dir, float fallOffStart, float fallOffEnd)
{
	m_Value.type = LIGHT_TYPE_SPOT;
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

void DX12Light::LightUpdate(ID3D12GraphicsCommandList* cmd, std::initializer_list<DX12DrawSet*> targets)
{
	if (CGH::GO.graphic.shadowMap && m_ShadowMap.get())
	{
		cmd->OMSetRenderTargets(0, nullptr, true, &m_ShadowMap->GetDsv());

		DX12_COMPUTE_CULLING_DESC culling = {};
		static const float radius = 500.0f;
		physx::PxMat44 shadowProj;
		physx::PxVec3 startPos;
		physx::PxVec3 dir;
		physx::PxVec2 shadowMapSize = m_ShadowMap->GetSize();
		DX12PSOAttributeNames psocon;

		psocon.dsvFormat = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		psocon.vs = DX12DrawSet::ShadowMapShaderCallName;
		psocon.ps = DX12DrawSet::ShadowMapShaderCallName;

		switch (m_Value.type)
		{
		case LIGHT_TYPE_DIRECTIONAL:
			XMStoreFloat4x4(shadowProj, XMMatrixOrthographicOffCenterLH(-radius, radius, -radius, radius, -radius, radius));
			break;
		case LIGHT_TYPE_POINT:
			XMStoreFloat4x4(shadowProj, XMMatrixPerspectiveFovLH(CGH::GO.graphic.fovAngleY, shadowMapSize.x / shadowMapSize.y, -m_Value.falloffEnd, m_Value.falloffEnd));
			culling.type = DX12_COMPUTE_CULLING_TYPE_SPHERE;
			culling.sphere.pos_Radian = physx::PxVec4(m_Value.position, m_Value.falloffEnd);
			break;
		case LIGHT_TYPE_SPOT:
			XMStoreFloat4x4(shadowProj, XMMatrixPerspectiveFovLH(CGH::GO.graphic.fovAngleY, 1.0, m_Value.falloffEnd * 0.02, m_Value.falloffEnd));
			culling.type = DX12_COMPUTE_CULLING_TYPE_CON;
			culling.con.dir_cos = physx::PxVec4(m_Value.direction, 0.1f);
			culling.con.pos_Length = physx::PxVec4(m_Value.position, m_Value.falloffEnd);
			break;
		default:
			break;
		}

		DX12ShadowMap::GetShadowMatrix(startPos, dir, shadowProj, m_Value.shadowMat);

		if (m_Value.type == LIGHT_TYPE_DIRECTIONAL)
		{
			for (auto& it : targets)
			{
				it->Draw(cmd, &psocon, nullptr);
			}
		}
		else
		{
			for (auto& it : targets)
			{
				it->Draw(cmd, &psocon, &culling);
			}
		}
	}
}
