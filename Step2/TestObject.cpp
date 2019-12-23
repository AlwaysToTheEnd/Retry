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
	RenderInfo renderInfo(RENDER_UI);
	renderInfo.texPoint.size = { 100.0f, 100.0f,0};
	renderInfo.meshOrTextureName = "ice.dds";
	auto transform = AddComponent<ComTransform>();
	AddComponent<ComRenderer>()->SetRenderInfo(renderInfo);

	auto uicol = AddComponent<ComUICollision>();
	PxTransform pos(150, 150, 0);
	pos.q = PxQuat(PxPiDivFour/4, PxVec3(0, 0, 1).getNormalized());
	transform->SetTransform(pos);
	uicol->SetSize({ 100.0f, 100.0f });
	uicol->AddFunc(std::bind(&TestObject::TextChange, this));

	font = AddComponent<ComFont>();
	font->SetFontName(L"baseFont.spritefont");
	font->m_Text = L"";
	font->m_OffsetPos.x = 100;
	font->m_OffsetPos.y = 150;
}

void TestObject::Update()
{

}

void TestObject::TextChange()
{
	static int i = 0;
	font->m_Text = L"Changed Text" + std::to_wstring(i++);
}
