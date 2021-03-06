#pragma once
#include <vector>
#include <functional>
#include <memory>
#include <foundation/PxMat44.h>
#include <foundation/PxTransform.h>
#include <geometry/PxGeometry.h>
#include "PhysicsObject.h"
#include "PhysXFunctionalObject.h"

namespace physx
{
	class PxRigidStatic;
	class PxRigidDynamic;
	class PxShape;
}

class DOCollsionMesh :public PhyscisObject
{
public:
	DOCollsionMesh(CGHScene& scene, GameObject* parent, const char* typeName, 
		physx::PxGeometryType::Enum type, physx::PxVec4 halfSize, bool isExclusive)
		: PhyscisObject(scene, parent, typeName)
		, m_Shape(nullptr)
		, m_Type(type)
		, m_HalfSize(halfSize)
		, m_IsExclusive(isExclusive)
	{
		
	}
	virtual ~DOCollsionMesh() = default;

	virtual void	Delete() override;

	bool			IsExclusive() const { return m_IsExclusive; }
	virtual void*	GetPxObject() override {return m_Shape;}

	void			SetTrigger(bool value);
	void			SetLocalPos(const physx::PxTransform& localPos);

private:
	virtual void	Update(float delta) override {}
	virtual void	Init(IGraphicDevice*, ISoundDevice*, PhysX4_1* physxDevice) override;

private:
	physx::PxGeometryType::Enum	m_Type;
	physx::PxVec4				m_HalfSize;
	bool						m_IsExclusive;
	physx::PxShape*				m_Shape;
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

	virtual void	Delete() override;

	virtual void*	GetPxObject() override { return m_RigidBody; }

	void			SetPos(const physx::PxTransform& pos);

	void			AddFunc(std::function<void()> func);
	bool			AttachCollisionMesh(PhyscisObject* mesh);
	bool			DetachCollisionMesh(PhyscisObject* mesh);

private:
	virtual void	Update(float delta) override;
	virtual void	Init(IGraphicDevice*, ISoundDevice*, PhysX4_1* physxDevice) override;

private:
	physx::PxRigidDynamic*					m_RigidBody;
	std::unique_ptr<PhysXFunctionalObject>	m_Funcs;
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

	virtual void	Delete() override;

	virtual void*	GetPxObject() override { return m_RigidBody; }

	void			SetPos(const physx::PxTransform& pos);

	bool			AttachCollisionMesh(PhyscisObject* mesh);
	bool			DetachCollisionMesh(PhyscisObject* mesh);

private:
	virtual void	Update(float delta) override;
	virtual void	Init(IGraphicDevice*, ISoundDevice*, PhysX4_1* physxDevice) override;

private:
	physx::PxRigidStatic* m_RigidBody;
};

class DOTransform :public DeviceObject
{
public:
	DOTransform(CGHScene& scene, GameObject* parent, const char* typeName)
		: DeviceObject(scene, parent, typeName)
		, m_Transform(physx::PxIDENTITY::PxIdentity)
		, m_Scale(1,1,1)
	{
		
	}
	virtual ~DOTransform() = default;

	physx::PxMat44				GetRTMatrix() const;
	const physx::PxVec3&		GetScale() const { return m_Scale; }
	const physx::PxTransform&	GetTransform() const { return m_Transform; }

	void						SetPosX(float x) { m_Transform.p.x = x; }
	void						SetPosY(float y) { m_Transform.p.y = y; }
	void						SetPosZ(float z) { m_Transform.p.z = z; }
	void						SetPos(const physx::PxVec3& pos) { m_Transform.p = pos; }
	void						SetScale(const physx::PxVec3& scale) { m_Scale = scale; }
	void						SetTransform(const physx::PxTransform& transform) { m_Transform = transform; }
	void						AddVector(const physx::PxVec3& vec) { m_Transform.p += vec; }

private:
	virtual void Update(float delta) override {}
	virtual void Init(IGraphicDevice*, ISoundDevice*, PhysX4_1* physxDevice) override {};

private:
	physx::PxTransform	m_Transform;
	physx::PxVec3		m_Scale;
};