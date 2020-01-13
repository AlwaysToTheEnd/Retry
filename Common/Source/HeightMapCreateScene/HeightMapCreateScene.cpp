#include "HeightMapCreateScene.h"
#include "../../Step2/TestObject.h"
#include  "GameCreator.h"

void HeightMapCreateScene::Init()
{
	CreateGameObject<GameCreator>();
}

bool HeightMapCreateScene::Update(const DirectX::Mouse::ButtonStateTracker& mouse, float delta)
{
	return CGHScene::Update(mouse, delta);
}
