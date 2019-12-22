#include "TestObject.h"
#include "BaseComponent.h"
#include "GraphicComponent.h"
#include "PxRigidDynamic.h"
#include "PxRigidStatic.h"
#include "PhysXFunctionalObject.h"
#include "d3dApp.h"
#include <random>
using namespace AniTree;
using namespace physx;

extern std::mt19937_64 g_random;

void TestObject::Init()
{
	RenderInfo renderInfo(RENDER_BOX);
	renderInfo.point.size = { 3.0f, 3.0f, 3.0f};
	renderInfo.point.color = {0,0,0,1};
	auto transform = AddComponent<ComTransform>();
	AddComponent<ComRenderer>()->SetRenderInfo(renderInfo);

	static float posx = 0;
	transform->SetTransform(PxTransform(posx, 3, 0));

	auto rigidCom = AddComponent<ComRigidDynamic>();
	auto rigidBody = rigidCom->GetRigidBody();
	auto funcs = new PhysXFunctionalObject();

	funcs->voidFuncs.push_back(std::bind(&TestObject::TextChange, this));
	rigidBody->userData = funcs;

	rigidBody->setGlobalPose(physx::PxTransform(physx::PxVec3(posx, 3, 0)));
	posx += 9;

	font = AddComponent<ComFont>();
	font->m_Text = L"";
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
		GetComponent<ComRigidDynamic>()->GetRigidBody()->addForce(
			physx::PxVec3(g_random()%10, g_random() % 10, g_random() % 10),PxForceMode::eVELOCITY_CHANGE);
	}

	if (GKEYBOARD.IsKeyPressed(KEYState::Up))
	{

	}
}

void TestObject::TextChange()
{
	font->m_Text = L"Changed Text" + std::to_wstring(font->GetID());
}
