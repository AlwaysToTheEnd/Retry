#include "PhysicsDO.h"
#include "GameObject.h"
#include "PxRigidStatic.h"
#include "PxRigidDynamic.h"
#include "PxScene.h"
#include "foundation/PxMat44.h"

void DORigidDynamic::Update(float delta)
{
	auto transform = m_Parent->GetComponent<DOTransform>();

	if (transform)
	{
		transform->SetTransform(m_RigidBody->getGlobalPose());
	}
}

void DORigidStatic::Update(float delta)
{
	auto transform = m_Parent->GetComponent<DOTransform>();

	if (transform)
	{
		transform->SetTransform(m_RigidBody->getGlobalPose());
	}
}

void DOUICollision::Update(float delta)
{
	auto transform = m_Parent->GetComponent<DOTransform>();

	if (transform)
	{
		physx::PxTransform offset(m_Offset.x, m_Offset.y, 0);
		physx::PxTransform wPos = transform->GetTransform();
		wPos = offset * wPos;
		m_ReservedUICol->push_back(UICollisions(m_Parent, wPos, m_Size, m_VoidFuncs));
	}
}
