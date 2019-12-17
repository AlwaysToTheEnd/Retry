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
	physx::PxDefaultAllocator					m_allocator;
	physx::PxDefaultErrorCallback				m_errorCallback;
	
	Px1RefPtr<physx::PxPhysics>					m_physics;
	Px1RefPtr<physx::PxFoundation>				m_foundation;
	Px1RefPtr<physx::PxDefaultCpuDispatcher>	m_dispatcher;
	Px1RefPtr<physx::PxCooking>					m_cooking;
	Px1RefPtr<physx::PxCudaContextManager>		m_cudaManager;
	Px1RefPtr<physx::PxScene>					m_scene;
	Px1RefPtr<physx::PxPvd>						m_PVD;

private:
	Px1RefPtr<physx::PxMaterial>				m_planeMaterial;
};

