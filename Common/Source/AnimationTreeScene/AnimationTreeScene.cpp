#include "AnimationTreeScene.h"
#include "../UIObjects/UIPanel.h"
#include "AnimationTreeCreator.h"

void AniTreeScene::Init()
{
	CreateGameObject<VisualizedAniTreeCreator>();
}

bool AniTreeScene::Update(const DirectX::Mouse::ButtonStateTracker& mouse, float delta)
{
	return CGHScene::Update(mouse, delta);
}
