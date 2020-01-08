#include "PhysicsDO.h"
#include "GameObject.h"
#include "PhysX4_1.h"

void DORigidDynamic::Delete()
{
	if (m_RigidBody)
	{
		m_RigidBody->getScene()->removeActor(*m_RigidBody);
		m_RigidBody = nullptr;
	}

	DeviceObject::Delete();
}

void DORigidDynamic::Update(float delta)
{
	auto transform = m_Parent->GetComponent<DOTransform>();

	if (transform)
	{
		transform->SetTransform(m_RigidBody->getGlobalPose());
	}
}

void DORigidDynamic::Init(PhysX4_1* physxDevice, IGraphicDevice*)
{

}

void DORigidStatic::Delete()
{
	if (m_RigidBody)
	{
		m_RigidBody->getScene()->removeActor(*m_RigidBody);
		m_RigidBody = nullptr;
	}

	DeviceObject::Delete();
}

void DORigidStatic::Update(float delta)
{
	auto transform = m_Parent->GetComponent<DOTransform>();

	if (transform)
	{
		transform->SetTransform(m_RigidBody->getGlobalPose());
	}
}

void DORigidStatic::Init(PhysX4_1* physxDevice, IGraphicDevice*)
{
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

void DOUICollision::Init(PhysX4_1* physxDevice, IGraphicDevice*)
{
	m_ReservedUICol = physxDevice->GetReservedUICollisionVector(m_Scene);
}

void DOMeshCollsion::Init(PhysX4_1* physxDevice, IGraphicDevice*)
{

}
