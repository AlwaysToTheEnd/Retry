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
}

void GameCreator::Init()
{
	SetUI();

	auto DLight = CreateComponenet<GameLight>();

	DLight->SetDirectionalLight({ 1,1,1 }, {0,-1,0});
}

void GameCreator::SetUI()
{
	auto clientSize = GETAPP->GetClientSize();
}

