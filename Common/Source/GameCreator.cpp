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

	CreateComponenet<HeightMapControlPanel>();
	auto DLight = CreateComponenet<GameLight>();
	auto pointLight = CreateComponenet<GameLight>();
	auto pointLight2 = CreateComponenet<GameLight>();

	DLight->SetDirectionalLight({ 1,1,1 }, { 0,-1,0 });
	pointLight->SetPointLight({ 0,0,1 }, { 40,5,40 }, 0, 15);
	pointLight2->SetPointLight({ 1,0,0 }, { 15,5,15 }, 0, 15);
}

void GameCreator::SetUI()
{
	auto clientSize = GETAPP->GetClientSize();
}

