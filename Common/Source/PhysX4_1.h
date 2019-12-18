#pragma once
#include "PxPhysicsAPI.h"
#include "IPhysicsDevice.h"
#include "Px1RefPtr.h"

class PhysX4_1 final : public IPhysicsDevice
{
public:
	PhysX4_1();
	virtual ~PhysX4_1();

	virtual bool Init(void* graphicDevicePtr);
	virtual void Update();
	virtual std::unique_ptr<IComponent> CreateComponent(COMPONENTTYPE type, GameObject& gameObject) override;

private: // Only Used by FuncPtr
	virtual void ComponentDeleteManaging(COMPONENTTYPE type, int id) override;

private:
	physx::PxFilterFlags ScissorFilter(physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0,
		physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1,
		physx::PxPairFlags& pairFlags, const void* constantBlock, physx::PxU32 constantBlockSize);

private:
	physx::PxDefaultAllocator					m_Allocator;
	physx::PxDefaultErrorCallback				m_ErrorCallback;
	
	Px1RefPtr<physx::PxFoundation>				m_Foundation;
	Px1RefPtr<physx::PxPhysics>					m_Physics;
	Px1RefPtr<physx::PxDefaultCpuDispatcher>	m_Dispatcher;
	Px1RefPtr<physx::PxCooking>					m_Cooking;
	Px1RefPtr<physx::PxCudaContextManager>		m_CudaManager;
	Px1RefPtr<physx::PxScene>					m_Scene;
	Px1RefPtr<physx::PxPvd>						m_PVD;

private:
	Px1RefPtr<physx::PxMaterial>				m_PlaneMaterial;
	InstanceAndIndexManager<Px1RefPtr<physx::PxRigidDynamic>>	m_Dynamics;
	InstanceAndIndexManager<Px1RefPtr<physx::PxRigidStatic>>	m_Statics;
};

