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

}

void GameCreator::Init()
{
	SetUI();
}

void GameCreator::SetUI()
{
	auto HMCP= CreateComponenet<HeightMapControlPanel>();
	
	auto clientSize = GETAPP->GetClientSize();
}

