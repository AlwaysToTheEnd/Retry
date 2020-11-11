#include "HeightMapCreateScene.h"
#include "../../Step2/TestObject.h"
#include  "GameCreator.h"

void HeightMapCreateScene::Init()
{
	CreateGameObject<GameCreator>();
}

void HeightMapCreateScene::Update(const DirectX::Mouse::ButtonStateTracker& mouse, float delta)
{
	CGHScene::Update(mouse, delta);
}
