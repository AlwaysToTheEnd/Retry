#include "GameCreator.h"
#include "GraphicDO.h"
#include "PhysicsDO.h"
#include "d3dApp.h"
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
}

void GameCreator::Init()
{
	SetUI();
	CreateComponenet<DOTransform>();
	auto light = CreateComponenet<DORenderer>();

	auto& rInfo = light->GetRenderInfo();
	rInfo.type = RENDER_LIGHT;
	rInfo.lightInfo.color = { 0.8f, 0.8f, 0.8f };
}

void GameCreator::SetUI()
{
	auto HMCP= CreateComponenet<HeightMapControlPanel>();
	
	auto clientSize = GETAPP->GetClientSize();
}

