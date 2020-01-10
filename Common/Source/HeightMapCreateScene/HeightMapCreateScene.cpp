#include "HeightMapCreateScene.h"
#include "../../Step2/TestObject.h"
#include "HeightMap.h"

void HeightMapCreateScene::Init()
{
	CreateGameObject<TestObject>();
	CreateGameObject<HeightMap>(L"../Common/HeightMap/HeightMap3.raw", physx::PxVec3(2.0f, 0.1f, 2.0f));
}

bool HeightMapCreateScene::Update(const DirectX::Mouse::ButtonStateTracker& mouse, float delta)
{
	return CGHScene::Update(mouse, delta);
}
