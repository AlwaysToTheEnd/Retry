#include "AnimationTreeScene.h"
#include "../UIObjects/UIPanel.h"

void AniTreeScene::Init()
{
	auto panel = AddGameObjects<UIPanel>();

	panel->SetBackGroundColor({ 0,0,1,1 });
	panel->SetSize(150, 150);
}

bool AniTreeScene::Update(const DirectX::Mouse::ButtonStateTracker& mouse)
{


	return CGHScene::Update(mouse);
}
