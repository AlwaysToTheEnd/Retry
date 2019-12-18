#include "BaseComponent.h"
#include "GameObject.h"
#include "PxRigidStatic.h"
#include "PxRigidDynamic.h"
#include "foundation/PxMat44.h"


void ComRigidDynamic::Update()
{
	auto transform = m_TargetGameObject->GetComponent<ComTransform>();

	if (transform)
	{
		transform->m_Transform = m_RigidBody->getGlobalPose();
	}
}

DirectX::XMFLOAT4X4 ComTransform::GetMatrix() const
{
	DirectX::XMFLOAT4X4 result;
	physx::PxMat44 pxMat(m_Transform);

	for (size_t i = 0; i < 4; i++)
	{
		for (size_t j = 0; j < 4; j++)
		{
			result.m[i][j] = pxMat[i][j];
		}
	}

	return result;
}


void ComRigidStatic::Update()
{
	auto transform = m_TargetGameObject->GetComponent<ComTransform>();

	if (transform)
	{
		transform->m_Transform = m_RigidBody->getGlobalPose();
	}
}
