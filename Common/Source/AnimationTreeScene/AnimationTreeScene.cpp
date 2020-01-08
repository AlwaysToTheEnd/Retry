#include "AnimationTreeScene.h"
#include "../UIObjects/UIPanel.h"
#include "AnimationTreeCreator.h"
#include "../../Step2/TestObject.h"
#include "HeightMap.h"

void AniTreeScene::Init()
{
	CreateGameObject<VisualizedAniTreeCreator>();
	CreateGameObject<TestObject>();
	CreateGameObject<HeightMap>(L"../Common/HeightMap/testRaw.raw");
}

bool AniTreeScene::Update(const DirectX::Mouse::ButtonStateTracker& mouse, float delta)
{
	return CGHScene::Update(mouse, delta);
}
