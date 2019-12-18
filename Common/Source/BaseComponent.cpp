#include "BaseComponent.h"

static void GetDXMatrixAtPxTransform(const physx::PxTransform& rigidActor, CGH::MAT16& mat)
{
	physx::PxMat44 pxMat(rigidActor);
	memcpy(&mat.m[0][0], &pxMat.column0.x, sizeof(CGH::MAT16));
}

ComRigidDynamic::ComRigidDynamic(GameObject& gameObject, int ID)
	:IComponent(COMPONENTTYPE::COM_DYNAMIC, gameObject, ID)
{
}

void ComRigidDynamic::Update()
{
}

void ComTransform::Update()
{
	GetDXMatrixAtPxTransform(m_Transform, m_Mat);
}

ComRigidStatic::ComRigidStatic(GameObject& gameObject, int ID)
	:IComponent(COMPONENTTYPE::COM_STATIC, gameObject, ID)
{
}

void ComRigidStatic::Update()
{
}
