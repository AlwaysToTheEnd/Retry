#pragma once
#include <vector>
#include <DirectXMath.h>
#include "IComponent.h"
#include "BaseClass.h"
#include "foundation/PxMat44.h"

class ComTransform :public IComponent
{
public:
	ComTransform(GameObject& gameObject, int ID, physx::PxTransform& transform)
		: IComponent(COMPONENTTYPE::COM_TRANSFORM, gameObject, ID)
		, m_Transform(transform)
	{
		
	}
	virtual ~ComTransform() = default;

	virtual void Update() override;

	const CGH::MAT16& GetMatrix() const { return m_Mat; }

private:
	physx::PxTransform& m_Transform;
	CGH::MAT16 m_Mat;
};

class ComRigidDynamic :public IComponent
{
public:
	ComRigidDynamic(GameObject& gameObject, int ID);
	virtual ~ComRigidDynamic() = default;

	virtual void Update() override;

private:
};

class ComRigidStatic :public IComponent
{
public:
	ComRigidStatic(GameObject& gameObject, int ID);
	virtual ~ComRigidStatic() = default;

	virtual void Update() override;

private:
};