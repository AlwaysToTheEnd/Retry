#include "AnimationTreeScene.h"
#include "AnimationTreeCreator.h"
#include "GameLight.h"
#include "../../Step2/TestObject.h"

void AniTreeScene::Init()
{
	CreateGameObject<VisualizedAniTreeCreator>();
	CreateGameObject<GameLight>()->SetDirectionalLight({ 1,1,1 }, { 0,-1,0 });
}

void AniTreeScene::Update(const DirectX::Mouse::ButtonStateTracker& mouse, float delta)
{
	CGHScene::Update(mouse, delta);
}
