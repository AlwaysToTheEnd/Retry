#pragma once
#include <vector>
#include <DirectXMath.h>
#include <functional>
#include "IComponent.h"
#include "BaseClass.h"
#include "PhysXFunctionalObject.h"
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

class ComUICollision :public IComponent
{
public:
	ComUICollision(GameObject& gameObject, int ID,
		std::vector<UICollisions>* reservedVec)
		:IComponent(COMPONENTTYPE::COM_UICOLLISTION, gameObject, ID)
		,m_Size(1,1)
		,m_ReservedUICol(reservedVec)
	{
		
	}
	virtual ~ComUICollision() = default;

	virtual void Update() override;
	void SetSize(const DirectX::XMFLOAT2& halfSize) { m_Size = halfSize; }
	void AddFunc(std::function<void()> func) { m_VoidFuncs.push_back(func); }

private:
	std::vector<UICollisions>* const	m_ReservedUICol;

	DirectX::XMFLOAT2					m_Size;
	std::vector<std::function<void()>>	m_VoidFuncs;
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

	physx::PxMat44 GetMatrix() const { return physx::PxMat44(m_Transform); }
	const physx::PxTransform& GetTransform() const { return m_Transform; }

private:
	physx::PxTransform m_Transform;
};