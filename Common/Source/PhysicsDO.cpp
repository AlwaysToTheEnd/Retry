#include "PhysicsDO.h"
#include "GameObject.h"
#include "PhysX4_1.h"

using namespace physx;

void DOCollsionMesh::Delete()
{
	if (m_Shape)
	{
		m_Shape->release();
		m_Shape = nullptr;
	}
}

void DOCollsionMesh::SetTrigger(bool value)
{
	m_Shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, value);
}

void DOCollsionMesh::SetLocalPos(const physx::PxTransform& localPos)
{
	m_Shape->setLocalPose(localPos);
}

void DOCollsionMesh::Init(IGraphicDevice*, ISoundDevice*, PhysX4_1* physxDevice)
{
	auto Physx = physxDevice->GetPhysics();

	switch (m_Type)
	{
	case physx::PxGeometryType::eSPHERE:
		m_Shape = Physx->createShape(PxSphereGeometry(m_HalfSize.x),
			*physxDevice->GetBaseMaterial(), m_IsExclusive);
		break;
	case physx::PxGeometryType::eCAPSULE:
		m_Shape = Physx->createShape(PxCapsuleGeometry(m_HalfSize.x, m_HalfSize.y),
			*physxDevice->GetBaseMaterial(), m_IsExclusive);
		break;
	case physx::PxGeometryType::eBOX:
		m_Shape = Physx->createShape(PxBoxGeometry(PxVec3(m_HalfSize.x, m_HalfSize.y, m_HalfSize.z)),
			*physxDevice->GetBaseMaterial(), m_IsExclusive);
		break;
	case physx::PxGeometryType::eCONVEXMESH:
		//#TODO
		assert(false);
		break;
	case physx::PxGeometryType::eTRIANGLEMESH:
		assert(false);
		//#TODO
		break;
	case physx::PxGeometryType::ePLANE:
	case physx::PxGeometryType::eHEIGHTFIELD:
	case physx::PxGeometryType::eGEOMETRY_COUNT:
	case physx::PxGeometryType::eINVALID:
	default:
		assert(false);
		break;
	}
}

void DORigidDynamic::Delete()
{
	if (m_RigidBody)
	{
		m_RigidBody->getScene()->removeActor(*m_RigidBody);
		m_RigidBody = nullptr;
	}

	DeviceObject::Delete();
}

void DORigidDynamic::SetPos(const physx::PxTransform& pos)
{
	m_RigidBody->setGlobalPose(pos);
}

void DORigidDynamic::AddFunc(std::function<void()> func)
{
	m_Funcs->m_VoidFuncs.push_back(func);
}

bool DORigidDynamic::AttachCollisionMesh(PhyscisObject* mesh)
{
	if (!mesh->Is<DOCollsionMesh>())
	{
		return false;
	}
	
	if (PxShape* shape = reinterpret_cast<PxShape*>(mesh->GetPxObject()))
	{
		bool result = m_RigidBody->attachShape(*shape);
		if (result)
		{
			result = PxRigidBodyExt::updateMassAndInertia(*m_RigidBody, 10.0f);
		}
		
		return result;
	}
	else
	{
		return false;
	}
}

bool DORigidDynamic::DetachCollisionMesh(PhyscisObject* mesh)
{
	if (!mesh->Is<DOCollsionMesh>())
	{
		return false;
	}

	if (PxShape* shape = reinterpret_cast<PxShape*>(mesh->GetPxObject()))
	{
		m_RigidBody->detachShape(*shape);
	}
	else
	{
		return false;
	}

	return true;
}

void DORigidDynamic::Update(float delta)
{
	auto transform = m_Parent->GetComponent<DOTransform>();

	if (transform)
	{
		transform->SetTransform(m_RigidBody->getGlobalPose());
	}
}

void DORigidDynamic::Init(IGraphicDevice*, ISoundDevice*, PhysX4_1* physxDevice)
{
	auto Physx = physxDevice->GetPhysics();

	m_RigidBody = Physx->createRigidDynamic(PxTransform(PxIdentity));
	physxDevice->GetScene(m_Scene)->addActor(*m_RigidBody);

	m_Funcs = std::make_unique<PhysXFunctionalObject>(this);
	m_RigidBody->userData = m_Funcs.get();
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

void DORigidStatic::SetPos(const physx::PxTransform& pos)
{
	m_RigidBody->setGlobalPose(pos);
}

bool DORigidStatic::AttachCollisionMesh(PhyscisObject* mesh)
{
	if (!mesh->Is<DOCollsionMesh>())
	{
		return false;
	}

	if (PxShape* shape = reinterpret_cast<PxShape*>(mesh->GetPxObject()))
	{
		return m_RigidBody->attachShape(*shape);
	}
	else
	{
		return false;
	}
}

bool DORigidStatic::DetachCollisionMesh(PhyscisObject* mesh)
{
	if (!mesh->Is<DOCollsionMesh>())
	{
		return false;
	}

	if (PxShape* shape = reinterpret_cast<PxShape*>(mesh->GetPxObject()))
	{
		m_RigidBody->detachShape(*shape);
	}
	else
	{
		return false;
	}

	return true;
}

void DORigidStatic::Update(float delta)
{
	auto transform = m_Parent->GetComponent<DOTransform>();

	if (transform)
	{
		transform->SetTransform(m_RigidBody->getGlobalPose());
	}
}

void DORigidStatic::Init(IGraphicDevice*, ISoundDevice*, PhysX4_1* physxDevice)
{
	auto Physx = physxDevice->GetPhysics();

	m_RigidBody = Physx->createRigidStatic(PxTransform(PxIdentity));
	physxDevice->GetScene(m_Scene)->addActor(*m_RigidBody);
}

physx::PxMat44 DOTransform::GetRTMatrix() const
{
	return physx::PxMat44(m_Transform);
}