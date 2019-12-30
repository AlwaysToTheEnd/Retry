#include "AnimationTreeScene.h"
#include "../UIObjects/UIPanel.h"
#include "AnimationTreeCreator.h"

void AniTreeScene::Init()
{
	auto treeCreater = AddGameObject<VisualizedAniTreeCreator>();

	treeCreater->SelectSkinnedData("zealot.X");
}

bool AniTreeScene::Update(const DirectX::Mouse::ButtonStateTracker& mouse, unsigned long long delta)
{
	return CGHScene::Update(mouse, delta);
}
