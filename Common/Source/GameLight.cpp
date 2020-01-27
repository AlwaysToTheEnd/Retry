#include "GameLight.h"
#include "GraphicDO.h"
#include "PhysicsDO.h"

void GameLight::SetDirectionalLight(const physx::PxVec3& color, const physx::PxVec3& dir)
{
	auto& renderInfo = m_Rrender->GetRenderInfo();
	renderInfo.lightInfo.type = LIGHT_TYPE_DIRECTIONAL;
	renderInfo.lightInfo.color = color;

	physx::PxVec3 normalDir = dir.getNormalized();
	physx::PxVec3 baseDir = { 0,0,1 };
	physx::PxVec3 crossVec = baseDir.cross(normalDir);
	
	m_Trans->SetTransform(physx::PxTransform(physx::PxQuat(baseDir.dot(normalDir), crossVec)));
}

void GameLight::SetPointLight(const physx::PxVec3& color, const physx::PxVec3& pos, float fallOffStart, float fallOffEnd)
{
	auto& renderInfo = m_Rrender->GetRenderInfo();
	renderInfo.lightInfo.type = LIGHT_TYPE_POINT;
	renderInfo.lightInfo.color = color;
	renderInfo.lightInfo.falloffAndPower.x = fallOffStart;
	renderInfo.lightInfo.falloffAndPower.y = fallOffEnd;

	m_Trans->SetPos(pos);
}

void GameLight::SetSpotLight(const physx::PxVec3& color, const physx::PxVec3& pos, float fallOffStart, float fallOffEndm, float angle)
{
}

void GameLight::Update(float delta)
{

}

void GameLight::Init()
{
	m_Trans = CreateComponenet<DOTransform>();
	m_Rrender = CreateComponenet<DORenderer>();

	m_Rrender->GetRenderInfo().type = RENDER_LIGHT;
}
