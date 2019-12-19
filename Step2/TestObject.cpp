#include "TestObject.h"
#include "BaseComponent.h"
#include "GraphicComponent.h"
#include "PxRigidDynamic.h"
#include "PxRigidStatic.h"
#include "PhysXFunctionalObject.h"
#include "d3dApp.h"

using namespace AniTree;

void TestObject::Init()
{
	AddComponent<ComTransform>();
	auto mesh = AddComponent<ComMesh>();
	AddComponent<ComRenderer>();
	ani= AddComponent<ComAnimator>();
	auto rigidCom = AddComponent<ComRigidDynamic>();
	auto rigidBody = rigidCom->GetRigidBody();
	auto funcs = new PhysXFunctionalObject();

	funcs->voidFuncs.push_back(std::bind(&TestObject::TextChange, this));
	rigidBody->userData = funcs;
	rigidBody->setGlobalPose(physx::PxTransform(physx::PxVec3(0, 3, 0)));
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

	aniTree->AddArrow(0, 1, TO_ANI_ARROW_TYPE::TO_ANI_NODE_TYPE_ONE_OK,
		CHANGE_CONDITION_TYPE_ANI_END, nullptr);
	aniTree->AddArrow(1, 2, TO_ANI_ARROW_TYPE::TO_ANI_NODE_TYPE_ONE_OK,
		CHANGE_CONDITION_TYPE_ANI_END, nullptr);
	aniTree->AddArrow(2, 0, TO_ANI_ARROW_TYPE::TO_ANI_NODE_TYPE_ONE_OK,
		CHANGE_CONDITION_TYPE_ANI_END, nullptr);

	font = AddComponent<ComFont>();
	font->m_Text = L"Test Font";
	font->m_OffsetPos.x = 100;
	font->m_OffsetPos.y = 150;
}

void TestObject::Update()
{
	if (GMOUSE.leftButton == MOUSEState::RELEASED)
	{
		GETAPP->ExcuteFuncOfClickedObjectFromPXDevice(100.0f);
	}

	static physx::PxTransform tr(physx::PxIDENTITY::PxIdentity);


	if (GKEYBOARD.IsKeyPressed(KEYState::Right))
	{

		tr.p.x += 0.1f;

		GetComponent<ComTransform>()->SetTransform(tr);
	}

	if (GKEYBOARD.IsKeyPressed(KEYState::Up))
	{

		tr.p.y += 0.1f;

		GetComponent<ComTransform>()->SetTransform(tr);
	}
}

void TestObject::TextChange()
{
	font->m_Text = L"Changed Text";
}
