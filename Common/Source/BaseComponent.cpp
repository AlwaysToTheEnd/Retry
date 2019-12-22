#include "BaseComponent.h"
#include "GameObject.h"
#include "PxRigidStatic.h"
#include "PxRigidDynamic.h"
#include "PxScene.h"
#include "foundation/PxMat44.h"


void ComRigidDynamic::Update()
{
	auto transform = m_TargetGameObject.GetComponent<ComTransform>();

	if (transform)
	{
		transform->SetTransform(m_RigidBody->getGlobalPose());
	}
}

void ComRigidStatic::Update()
{
	auto transform = m_TargetGameObject.GetComponent<ComTransform>();

	if (transform)
	{
		transform->SetTransform(m_RigidBody->getGlobalPose());
	}
}
