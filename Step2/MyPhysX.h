#pragma once
#include "PxPhysicsAPI.h"
#include "Px1RefPtr.h"

class MyPhysX
{
public:
	MyPhysX();
	~MyPhysX();

	void InitPhsyX();

private:
	physx::PxDefaultAllocator				m_allocator;
	physx::PxDefaultErrorCallback			m_errorCallback;

	Px1RefPtr<physx::PxPhysics>				m_physics;
	Px1RefPtr<physx::PxFoundation>			m_foundation;
	Px1RefPtr<physx::PxDefaultCpuDispatcher>m_dispatcher;
	Px1RefPtr<physx::PxCooking>				m_cooking;
	Px1RefPtr<physx::PxCudaContextManager>	m_cudaManager;
	Px1RefPtr<physx::PxScene>				m_scene;
	Px1RefPtr<physx::PxPvd>					m_PVD;


private:
	Px1RefPtr<physx::PxMaterial>			m_planeMaterial;
};

