#include "AnimationTreeScene.h"
#include "../UIObjects/UIPanel.h"
#include "AnimationTreeCreator.h"
#include "../../Step2/TestObject.h"

void AniTreeScene::Init()
{
	CreateGameObject<VisualizedAniTreeCreator>();
	CreateGameObject<TestObject>();
}

bool AniTreeScene::Update(const DirectX::Mouse::ButtonStateTracker& mouse, float delta)
{
	return CGHScene::Update(mouse, delta);
}
