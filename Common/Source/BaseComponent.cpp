#include "BaseComponent.h"
#include "GameObject.h"
#include "PxRigidStatic.h"
#include "PxRigidDynamic.h"
#include "PxScene.h"
#include "foundation/PxMat44.h"

void ComRigidDynamic::Update(unsigned long long delta)
{
	auto transform = m_TargetGameObject.GetComponent<ComTransform>();

	if (transform)
	{
		transform->SetTransform(m_RigidBody->getGlobalPose());
	}
}

void ComRigidStatic::Update(unsigned long long delta)
{
	auto transform = m_TargetGameObject.GetComponent<ComTransform>();

	if (transform)
	{
		transform->SetTransform(m_RigidBody->getGlobalPose());
	}
}

void ComUICollision::Update(unsigned long long delta)
{
	auto transform = m_TargetGameObject.GetComponent<ComTransform>();

	if (transform)
	{
		physx::PxTransform wPos = transform->GetTransform();
		wPos.p.x += m_Offset.x;
		wPos.p.y += m_Offset.y;
		m_ReservedUICol->push_back(UICollisions(&m_TargetGameObject, wPos, m_Size, m_VoidFuncs));
	}
}
