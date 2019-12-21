#include "TestObject.h"
#include "BaseComponent.h"
#include "GraphicComponent.h"
#include "PxRigidDynamic.h"
#include "PxRigidStatic.h"
#include "PhysXFunctionalObject.h"
#include "d3dApp.h"

using namespace AniTree;
using namespace physx;

void TestObject::Init()
{
	RenderInfo renderInfo(RENDER_TEX_PLANE);
	renderInfo.texPoint.size = { 5.0f, 3.0f, 0};
	renderInfo.meshOrTextureName = "ice.dds";
	AddComponent<ComTransform>();
	auto mesh = AddComponent<ComMesh>();
	AddComponent<ComRenderer>()->SetRenderInfo(renderInfo);
	ani= AddComponent<ComAnimator>();
	auto rigidCom = AddComponent<ComRigidStatic>();
	auto rigidBody = rigidCom->GetRigidBody();
	auto funcs = new PhysXFunctionalObject();

	funcs->voidFuncs.push_back(std::bind(&TestObject::TextChange, this));
	rigidBody->userData = funcs;
	rigidBody->setGlobalPose(physx::PxTransform(physx::PxVec3(0, 1, 0)));
	std::vector<std::string> names;
	mesh->GetMeshNames(names);

	if (names.size())
	{
		mesh->SelectMesh(names.front());
	}

	names.clear();
	ani->GetSkinNames(names);

	if (names.size())
	{
		ani->SelectSkin(names.front());
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
		GETAPP->ExcuteFuncOfClickedObjectFromPXDevice(1000.0f);
	}

	if (GKEYBOARD.IsKeyPressed(KEYState::Right))
	{
		GetComponent<ComRigidDynamic>()->GetRigidBody()->addForce(physx::PxVec3(0, 35, 0),PxForceMode::eVELOCITY_CHANGE);
	}

	if (GKEYBOARD.IsKeyPressed(KEYState::Up))
	{

	}
}

void TestObject::TextChange()
{
	font->m_Text = L"Changed Text";
}
