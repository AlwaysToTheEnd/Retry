#include "GameCreator.h"
#include "GraphicDO.h"
#include "PhysicsDO.h"
#include "d3dApp.h"
#include "GameLight.h"
#include "../../UIObjects/HeightMapControlPanel.h"

void GameCreator::Delete()
{
	
}

void GameCreator::Update(float delta)
{
	auto trans = GetComponent<DOTransform>();
	static float angle = 0;
	angle += delta;

	physx::PxQuat rotation(angle, physx::PxVec3(1, 0, 0));
	trans->SetTransform(physx::PxTransform(rotation));

	GetComponent<GameLight>()->GetComponent<DOTransform>()->AddVector(physx::PxVec3(0, 0, delta));
}

void GameCreator::Init()
{
	SetUI();
	CreateComponenet<DOTransform>();
	auto light = CreateComponenet<DORenderer>();

	auto& rInfo = light->GetRenderInfo();
	rInfo.type = RENDER_LIGHT;
	rInfo.lightInfo.color = { 0.8f, 0.8f, 0.8f };

	auto pointLight = CreateComponenet<GameLight>();

	pointLight->SetPointLight({ 0,1,1 }, { 0,3,3 }, 1, 15);
}

void GameCreator::SetUI()
{
	auto HMCP= CreateComponenet<HeightMapControlPanel>();
	
	auto clientSize = GETAPP->GetClientSize();
}

