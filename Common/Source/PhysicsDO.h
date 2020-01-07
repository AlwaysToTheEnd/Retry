#pragma once
#include <vector>
#include <DirectXMath.h>
#include <functional>
#include "DeviceObject.h"
#include "BaseClass.h"
#include "PhysXFunctionalObject.h"
#include "foundation/PxTransform.h"

namespace physx
{
	class PxRigidStatic;
	class PxRigidDynamic;
}

class DORigidDynamic :public DeviceObject
{
public:
	DORigidDynamic(CGHScene& scene, GameObject* const parent, unsigned int hashCode)
		: DeviceObject(scene, parent, hashCode)
		, m_RigidBody(rigidBody)
	{

	}
	virtual ~DORigidDynamic() = default;

	virtual void Update(float delta) override;
	physx::PxRigidDynamic* GetRigidBody() { return m_RigidBody; }

private:
	physx::PxRigidDynamic* m_RigidBody;
};

class DORigidStatic :public DeviceObject
{
public:
	DORigidStatic(CGHScene& scene, GameObject* const parent, unsigned int hashCode)
		: DeviceObject(scene, parent, hashCode)
		, m_RigidBody(rigidBody)
	{

	}
	virtual ~DORigidStatic() = default;

	virtual void Update(float delta) override;
	physx::PxRigidStatic* GetRigidBody() { return m_RigidBody; }

private:
	physx::PxRigidStatic* m_RigidBody;
};

class DOUICollision :public DeviceObject
{
public:
	DOUICollision(CGHScene& scene, GameObject* const parent, unsigned int hashCode)
		: DeviceObject(scene, parent, hashCode)
		,m_Size(1,1)
		,m_Offset(0,0)
		,m_ReservedUICol(reservedVec)
	{
		
	}
	virtual ~DOUICollision() = default;

	virtual void Update(float delta) override;

	void SetSize(const physx::PxVec2& halfSize) { m_Size = halfSize; }
	void SetOffset(const physx::PxVec2& offset) { m_Offset = offset; }
	const physx::PxVec2& GetSize() const { return m_Size; }
	void AddFunc(std::function<void()> func) { m_VoidFuncs.push_back(func); }

private:
	std::vector<UICollisions>* const		m_ReservedUICol;

	physx::PxVec2							m_Size;
	physx::PxVec2							m_Offset;
	std::vector<std::function<void()>>		m_VoidFuncs;
};

class DOTransform :public DeviceObject
{
public:
	DOTransform(CGHScene& scene, GameObject* const parent, unsigned int hashCode)
		: DeviceObject(scene, parent, hashCode)
		, m_Transform(physx::PxIDENTITY::PxIdentity)
	{
		
	}
	virtual ~DOTransform() = default;

	virtual void Update(float delta) override {}
	void SetTransform(const physx::PxTransform& transform) { m_Transform = transform; }
	void SetPosX(float x) { m_Transform.p.x = x; }
	void SetPosY(float y) { m_Transform.p.y = y; }
	void SetPosZ(float z) { m_Transform.p.z = z; }
	void AddVector(physx::PxVec3 vec) { m_Transform.p += vec; }
	void SetPos(physx::PxVec3 pos) { m_Transform.p = pos; }

	physx::PxMat44 GetMatrix() const { return physx::PxMat44(m_Transform); }
	const physx::PxTransform& GetTransform() const { return m_Transform; }

private:
	physx::PxTransform m_Transform;
};