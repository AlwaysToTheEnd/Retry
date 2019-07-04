#pragma once
#include "PhysicsDevice.h"
#include "PxPhysicsAPI.h"
#include "Px1RefPtr.h"

class MyGPULoadHook :public PxGpuLoadHook
{
public:
	MyGPULoadHook() = default;
	virtual ~MyGPULoadHook() = default;

	virtual const char* getPhysXGpuDllName() const override
	{
		return "../Out/PhysXGpu_64.dll";
	}
};

class PhysX4_0 final : public PhysicsDevice
{
public:
	PhysX4_0();
	virtual ~PhysX4_0();

	virtual bool Init();
	virtual void Update();

private:
	static physx::PxFilterFlags ScissorFilter(physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0,
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

