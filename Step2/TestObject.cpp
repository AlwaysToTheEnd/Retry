#include "TestObject.h"
#include "BaseComponent.h"
#include "GraphicComponent.h"
#include "d3dApp.h"

using namespace AniTree;

void TestObject::Init()
{
	AddComponent<ComTransform>();
	auto mesh = AddComponent<ComMesh>();
	AddComponent<ComRenderer>();
	ani= AddComponent<ComAnimator>();
	
	std::vector<std::string> names;
	mesh->GetMeshNames(names);

	if (names.size())
	{
		mesh->SelectMesh(names.back());
	}

	names.clear();
	ani->GetSkinNames(names);

	if (names.size())
	{
		ani->SelectSkin(names.back());
	}
	
	names.clear();
	ani->GetAniNames(names);

	if (names.size())
	{
		ani->SelectAnimation(names.front());
	}

	ani->SetAnimationTree(true);

	AnimationTree* aniTree = ani->GetAnimationTree();

	for (size_t i = 0; i < 3; i++)
	{
		aniTree->AddAniNode(names[i], ani->GetAniEndTime(names[i]), false);
	}

	UnionData standard;
	standard._b = true;
	TriggerData trigger(static_cast<TRIGGER_TYPE>((TRIGGER_TYPE_SAME)), 
		DATA_TYPE::TYPE_BOOL, standard);

	aniTree->AddArrow(0, 1, TO_ANI_ARROW_TYPE::TO_ANI_NODE_TYPE_ONE_OK,
		CHANGE_CONDITION_TYPE_TRIGGER, &trigger);
	aniTree->AddArrow(1, 2, TO_ANI_ARROW_TYPE::TO_ANI_NODE_TYPE_ONE_OK,
		CHANGE_CONDITION_TYPE_TRIGGER, &trigger);
	aniTree->AddArrow(2, 0, TO_ANI_ARROW_TYPE::TO_ANI_NODE_TYPE_ONE_OK,
		CHANGE_CONDITION_TYPE_TRIGGER, &trigger);
}

void TestObject::Update()
{
	AnimationTree* aniTree = ani->GetAnimationTree();

	std::vector<OutputTrigger> triggers;
	aniTree->GetTriggers(triggers);

	for (int i = 0; i < 3; i++)
	{
		if(GKEYBOARD.IsKeyPressed(static_cast<DirectX::Keyboard::Keys>(DirectX::Keyboard::D1 + i)))
		{
			triggers[i].trigger.m_Trigger._b = true;
			break;
		}
	}
}
