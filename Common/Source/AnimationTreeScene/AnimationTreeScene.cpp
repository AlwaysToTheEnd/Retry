#include "AnimationTreeScene.h"
#include "../UIObjects/UIPanel.h"
#include "AnimationTreeCreator.h"
#include "../../Step2/TestObject.h"
#include "HeightMap.h"

void AniTreeScene::Init()
{
	CreateGameObject<VisualizedAniTreeCreator>();
	CreateGameObject<TestObject>();
	CreateGameObject<HeightMap>(L"../Common/HeightMap/heightMap1.raw", physx::PxVec3(0.5f, 0.5f, 0.5f));
}

bool AniTreeScene::Update(const DirectX::Mouse::ButtonStateTracker& mouse, float delta)
{
	return CGHScene::Update(mouse, delta);
}
