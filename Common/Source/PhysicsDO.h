#pragma once
#include <vector>
#include <functional>
#include <foundation/PxMat44.h>
#include <foundation/PxTransform.h>
#include "PhysXFunctionalObject.h"
#include "PhysicsObject.h"

namespace physx
{
	class PxRigidStatic;
	class PxRigidDynamic;
	class PxShape;
}

class DOMeshCollsion :public PhyscisObject
{
public:
	DOMeshCollsion(CGHScene& scene, GameObject* parent, const char* typeName)
		: PhyscisObject(scene, parent, typeName)
		, m_Shape(nullptr)
	{
		
	}

private:
	virtual void Update(float delta) override {}
	virtual void Init(PhysX4_1* physxDevice, IGraphicDevice*);

private:
	physx::PxShape* m_Shape;
};

class DORigidDynamic :public PhyscisObject
{
public:
	DORigidDynamic(CGHScene& scene, GameObject* parent, const char* typeName)
		: PhyscisObject(scene, parent, typeName)
		, m_RigidBody(nullptr)
	{

	}
	virtual ~DORigidDynamic() = default;

	virtual void Delete() override;

private:
	virtual void Update(float delta) override;
	virtual void Init(PhysX4_1* physxDevice, IGraphicDevice*);

private:
	physx::PxRigidDynamic* m_RigidBody;
};

class DORigidStatic :public PhyscisObject
{
public:
	DORigidStatic(CGHScene& scene, GameObject* parent, const char* typeName)
		: PhyscisObject(scene, parent, typeName)
		, m_RigidBody(nullptr)
	{

	}
	virtual ~DORigidStatic() = default;

	virtual void Delete() override;

private:
	virtual void Update(float delta) override;
	virtual void Init(PhysX4_1* physxDevice, IGraphicDevice*);

private:
	physx::PxRigidStatic* m_RigidBody;
};

class DOUICollision :public DeviceObject
{
public:
	DOUICollision(CGHScene& scene, GameObject* parent, const char* typeName)
		: DeviceObject(scene, parent, typeName)
		, m_ReservedUICol(nullptr)
		, m_Size(1,1)
		, m_Offset(0,0)
	{
	}
	virtual ~DOUICollision() = default;

	const physx::PxVec2&	GetSize() const { return m_Size; }

	void					SetSize(const physx::PxVec2& halfSize) { m_Size = halfSize; }
	void					SetOffset(const physx::PxVec2& offset) { m_Offset = offset; }
	void					AddFunc(std::function<void()> func) { m_VoidFuncs.push_back(func); }

private:
	virtual void Update(float delta) override;
	virtual void Init(PhysX4_1* physxDevice, IGraphicDevice*);

private:
	std::vector<UICollisions>* 				m_ReservedUICol;

	physx::PxVec2							m_Size;
	physx::PxVec2							m_Offset;
	std::vector<std::function<void()>>		m_VoidFuncs;
};

class DOTransform :public DeviceObject
{
public:
	DOTransform(CGHScene& scene, GameObject* parent, const char* typeName)
		: DeviceObject(scene, parent, typeName)
		, m_Transform(physx::PxIDENTITY::PxIdentity)
	{
		
	}
	virtual ~DOTransform() = default;

	physx::PxMat44				GetMatrix() const { return physx::PxMat44(m_Transform); }
	const physx::PxTransform&	GetTransform() const { return m_Transform; }

	void						SetPosX(float x) { m_Transform.p.x = x; }
	void						SetPosY(float y) { m_Transform.p.y = y; }
	void						SetPosZ(float z) { m_Transform.p.z = z; }
	void						SetPos(physx::PxVec3 pos) { m_Transform.p = pos; }
	void						SetTransform(const physx::PxTransform& transform) { m_Transform = transform; }
	void						AddVector(physx::PxVec3 vec) { m_Transform.p += vec; }

private:
	virtual void Update(float delta) override {}
	virtual void Init(PhysX4_1*, IGraphicDevice*) {};

private:
	physx::PxTransform m_Transform;
};