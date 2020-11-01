#include "TestObject.h"
#include "GraphicDO.h"
#include "PhysicsDO.h"
#include "PxRigidDynamic.h"

void TestObject::Update(float delta)
{

}

void TestObject::Init()
{
	/*auto trans = CreateComponenet<DOTransform>();
	auto mesh = CreateComponenet<DORenderMesh>();
	auto Renderer = CreateComponenet<DORenderer>();
	auto Animator = CreateComponenet<DOAnimator>();

	std::vector<std::string> skins;
	std::vector<std::string> anis;

	Renderer->SetRenderInfo(RenderInfo(RENDER_SKIN));
	Animator->GetSkinNames(skins);
	Animator->SelectSkin(skins[0]);
	mesh->SelectMesh(CGH::SKINNED_MESH, skins[0]);
	Animator->GetAniNames(anis);
	Animator->SelectAnimation(anis[0]);

	trans->SetPos({ 0,0,0 });*/
}

void TestObject::TestClickedFunc()
{

}
