#include "PhysX4_1.h"
#include "PhysicsDO.h"
#include "CGHScene.h"
#include "d3dApp.h"
#include <Windows.h>

using namespace physx;

PhysX4_1::PhysX4_1()
	: m_GraphicsDevice(nullptr)
	, m_PickingPos(0,0,0)
{
}

PhysX4_1::~PhysX4_1()
{
	m_Material = nullptr;
	m_Scenes.clear();
	m_Dispatcher = nullptr;
	m_Cooking = nullptr;
	m_CudaManager = nullptr;
	m_Physics = nullptr;

	if (m_PVD.Get())
	{
		PxPvdTransport* transport = m_PVD->getTransport();
		m_PVD = nullptr;
		transport->release();
	}
	else
	{
		m_PVD = nullptr;
	}

	m_Foundation = nullptr;
	m_Foundation = nullptr;
}

bool PhysX4_1::Init(void* graphicDevicePtr)
{
	m_Foundation = PxCreateFoundation(PX_PHYSICS_VERSION,
		m_Allocator, m_ErrorCallback);

	m_PVD = PxCreatePvd(*m_Foundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
	m_PVD->connect(*transport, PxPvdInstrumentationFlag::eALL);

	m_Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_Foundation,
		PxTolerancesScale(),
		true, m_PVD.Get());
	PxRegisterHeightFields(*m_Physics);

	SYSTEM_INFO info;
	GetSystemInfo(&info);

	m_Dispatcher = PxDefaultCpuDispatcherCreate(info.dwNumberOfProcessors);
	m_GraphicsDevice = graphicDevicePtr;

	m_Cooking = PxCreateCooking(PX_PHYSICS_VERSION, *m_Foundation,
		PxCookingParams(PxTolerancesScale()));
	
	m_Material = m_Physics->createMaterial(0.5f, 0.5f, 0.6f);

	return true;
}

void PhysX4_1::Update(const CGHScene& scene)
{
	auto currScene = m_Scenes.find(scene.GetSceneName());

	currScene->second.scene->simulate(1.0f / 60.0f);
	currScene->second.scene->fetchResults(true);
}

void PhysX4_1::CreateScene(const CGHScene& scene)
{
	assert(m_Scenes.find(scene.GetSceneName()) == m_Scenes.end());

	PxSceneDesc sceneDesc(m_Physics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	sceneDesc.cpuDispatcher = m_Dispatcher.Get();
	sceneDesc.broadPhaseType = PxBroadPhaseType::eABP;
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;

	if (m_GraphicsDevice != nullptr)
	{
		PxCudaContextManagerDesc cudaDesc;
		cudaDesc.graphicsDevice = m_GraphicsDevice;
		m_CudaManager = PxCreateCudaContextManager(*m_Foundation, cudaDesc);
		sceneDesc.gpuDispatcher = m_CudaManager->getGpuDispatcher();
		sceneDesc.flags |= PxSceneFlag::eENABLE_PCM;
		sceneDesc.broadPhaseType = PxBroadPhaseType::eGPU;
		sceneDesc.gpuMaxNumPartitions = 8;
	}

	auto addedScene = m_Physics->createScene(sceneDesc);

	if (m_Scenes.size() == 0)
	{
		PxPvdSceneClient* pvdClient = addedScene->getScenePvdClient();
		if (pvdClient)
		{
			pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
			pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
			pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
			addedScene->setVisualizationParameter(PxVisualizationParameter::eACTOR_AXES, 2.0f);
		}
	}

	m_Scenes[scene.GetSceneName()].scene = addedScene;
}

void PhysX4_1::RegisterDeviceObject(CGHScene& scene, DeviceObject* gameObject)
{
	if (gameObject->IsObjectType(GameObject::PHYSICS_OBJECT))
	{
		
	}
}

void PhysX4_1::UnRegisterDeviceObject(CGHScene& scene, DeviceObject* gameObject)
{
	if (gameObject->IsObjectType(GameObject::PHYSICS_OBJECT))
	{
		
	}
}

physx::PxScene* PhysX4_1::GetScene(CGHScene& scene)
{
	return m_Scenes[scene.GetSceneName()].scene.Get();
}

PxFilterFlags PhysX4_1::ScissorFilter(PxFilterObjectAttributes attributes0,
	PxFilterData filterData0, PxFilterObjectAttributes attributes1,
	PxFilterData filterData1, PxPairFlags& pairFlags,
	const void* constantBlock, PxU32 constantBlockSize)
{
	return PxFilterFlags();
}