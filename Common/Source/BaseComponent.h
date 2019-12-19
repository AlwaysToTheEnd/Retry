#pragma once
#include <vector>
#include <DirectXMath.h>
#include "IComponent.h"
#include "BaseClass.h"
#include "foundation/PxTransform.h"

namespace physx
{
	class PxRigidStatic;
	class PxRigidDynamic;
}

class ComRigidDynamic :public IComponent
{
public:
	ComRigidDynamic(GameObject& gameObject, int ID,
		physx::PxRigidDynamic* rigidBody)
		:IComponent(COMPONENTTYPE::COM_DYNAMIC, gameObject, ID)
		, m_RigidBody(rigidBody)
	{

	}
	virtual ~ComRigidDynamic() = default;

	virtual void Update() override;
	physx::PxRigidDynamic* GetRigidBody() { return m_RigidBody; }

private:
	physx::PxRigidDynamic* m_RigidBody;
};

class ComRigidStatic :public IComponent
{
public:
	ComRigidStatic(GameObject& gameObject, int ID,
		physx::PxRigidStatic* rigidBody)
		:IComponent(COMPONENTTYPE::COM_STATIC, gameObject, ID)
		, m_RigidBody(rigidBody)
	{

	}
	virtual ~ComRigidStatic() = default;

	virtual void Update() override;
	physx::PxRigidStatic* GetRigidBody() { return m_RigidBody; }

private:
	physx::PxRigidStatic* m_RigidBody;
};

class ComTransform :public IComponent
{
public:
	ComTransform(GameObject& gameObject, int ID)
		: IComponent(COMPONENTTYPE::COM_TRANSFORM, gameObject, ID)
		, m_Transform(physx::PxIDENTITY::PxIdentity)
	{
		
	}
	virtual ~ComTransform() = default;

	virtual void Update() override {}
	void SetTransform(const physx::PxTransform& transform) { m_Transform = transform; }

	DirectX::XMFLOAT4X4 GetMatrix() const;

private:
	physx::PxTransform m_Transform;
};