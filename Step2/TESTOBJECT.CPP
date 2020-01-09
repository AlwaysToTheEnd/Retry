#include "TestObject.h"
#include "GraphicDO.h"
#include "PhysicsDO.h"
#include "PxRigidDynamic.h"

void TestObject::Update(float delta)
{
}

void TestObject::Init()
{
	CreateComponenet<DOTransform>();
	auto colMesh = CreateComponenet<DOCollsionMesh>(true, physx::PxGeometryType::eBOX, physx::PxVec4(1,1,1,0), true);
	auto rigid = CreateComponenet<DORigidDynamic>();

	rigid->AttachCollisionMesh(colMesh);
	rigid->SetPos(physx::PxTransform(5, 10, 5));
	rigid->AddFunc(std::bind(&TestObject::TestClickedFunc, this));

	auto renderer = CreateComponenet<DORenderer>();

	RenderInfo renderInfo(RENDER_BOX);
	renderInfo.point.color = { 0,1,0,1 };
	renderInfo.point.size = { 1,1,1 };

	renderer->SetRenderInfo(renderInfo);
}

void TestObject::TestClickedFunc()
{
	auto rigid = reinterpret_cast<physx::PxRigidDynamic*> (GetComponent<DORigidDynamic>()->GetPxObject());

	rigid->addForce(physx::PxVec3(3, 5, 3), physx::PxForceMode::eVELOCITY_CHANGE);
}
