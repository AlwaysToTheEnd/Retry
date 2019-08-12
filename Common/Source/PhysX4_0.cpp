#include "PhysX4_0.h"
#include <Windows.h>
using namespace physx;


PhysX4_0::PhysX4_0()
{
}

PhysX4_0::~PhysX4_0()
{
	m_planeMaterial = nullptr;
	m_scene = nullptr;
	m_dispatcher = nullptr;
	m_cooking = nullptr;
	m_cudaManager = nullptr;
	m_physics = nullptr;

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
	

	m_foundation = nullptr;
	m_foundation = nullptr;
}

bool PhysX4_0::Init(void* graphicDevicePtr)
{
	// ���� ���ʰ� �Ǵ� Foundation ����, �̱��� Ŭ�����ӿ� ��������.
	m_foundation = PxCreateFoundation(PX_PHYSICS_VERSION,
		m_allocator, m_errorCallback);

	// Physics Visual Debugger�� �����ϱ� ��Ʈ ������ ���� �۾�. 
	// ������� �ʴ´ٸ� ���� �ʾƵ� �����ϴ�.

	m_PVD = PxCreatePvd(*m_foundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
	m_PVD->connect(*transport, PxPvdInstrumentationFlag::eALL);

	// ��� Physics ���ҽ� ������ ���� ���� Ŭ���� ����, �̱��� Ŭ�����ӿ� ��������.
	// DirectX Device�� ���� �������� �����ϸ� ����.
	// PxTolerancesScale ��� ����ó���� �־ ������ �Ǵ� ���� ����.
	// length(1) 
	// mass(1000) 
	// speed(10) 

	m_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_foundation,
		PxTolerancesScale(),
		true, m_PVD.Get());
	PxRegisterHeightFields(*m_physics);

	SYSTEM_INFO info;
	GetSystemInfo(&info);

	// CPU�۾��� ������� ���� ���Ѵ�. ���� ����
	// �������� ���� ���Ѵ�.
	m_dispatcher = PxDefaultCpuDispatcherCreate(info.dwNumberOfProcessors);

	// ������Ʈ���� �ùķ��̼� �� ������ �����Ѵ�.
	// �� �ȿ� �ùķ��̼� �� ���� �� �� ���� ���� �������� �ùķ��̼� �Ѵ�.
	// PxSceneDesc�� ���� �Ӽ� ������ ���� ����ü�̴�.
	PxSceneDesc sceneDesc(m_physics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	sceneDesc.cpuDispatcher = m_dispatcher.Get();
	sceneDesc.broadPhaseType = PxBroadPhaseType::eABP;
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;

	if (graphicDevicePtr != nullptr)
	{
		PxCudaContextManagerDesc cudaDesc;
		cudaDesc.graphicsDevice = graphicDevicePtr;
		m_cudaManager = PxCreateCudaContextManager(*m_foundation, cudaDesc);
		sceneDesc.gpuDispatcher = m_cudaManager->getGpuDispatcher();
		sceneDesc.flags |= PxSceneFlag::eENABLE_GPU_DYNAMICS;
		sceneDesc.flags |= PxSceneFlag::eENABLE_PCM;
		sceneDesc.flags |= PxSceneFlag::eENABLE_STABILIZATION;
		sceneDesc.broadPhaseType = PxBroadPhaseType::eGPU;
		sceneDesc.gpuMaxNumPartitions = 8;
	}

	// �������� �ùķ��̼�,���� �̺�Ʈ���� �ݹ� �������̽��� �����Ѵ�

	// �ݹ��Ģ
	// �ݹ��� ���ν����� �Ǵ� �ùķ��̼� �����忡�� ���ÿ� ����� �� �����Ƿ�
	// SDK�� ���¸� �����ϸ� �ȵǸ� Ư�� ��ü�� �����ϰų� �ı��ؼ��� �ȵȴ�
	// ���� ������ �ʿ��� ��� ���� ������ ���ۿ� ���� �� �ùķ��̼� �ܰ� ���� ������ ��
	//static SceneSimulationEventCallBack custumSimulationEvent;
	//sceneDesc.simulationEventCallback = &custumSimulationEvent;

	m_scene = m_physics->createScene(sceneDesc);

	//pvd Ŭ���̾�Ʈ ����
	PxPvdSceneClient* pvdClient = m_scene->getScenePvdClient();
	if (pvdClient)
	{
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
		m_scene->setVisualizationParameter(PxVisualizationParameter::eACTOR_AXES, 2.0f);
	}

	// �뷮�� �����͸� �����ϰ� ��ȯ, ����ȭ �ϴ� ��ƿ��Ƽ ����
	m_cooking = PxCreateCooking(PX_PHYSICS_VERSION, *m_foundation,
		PxCookingParams(PxTolerancesScale()));

	return true;
}

void PhysX4_0::Update()
{
	m_scene->simulate(1.0f / 60.0f);
	m_scene->fetchResults(true);
}

std::shared_ptr<IComponent> PhysX4_0::CreateComponent(PxTransform& trans)
{

	return nullptr;
}

PxFilterFlags PhysX4_0::ScissorFilter(PxFilterObjectAttributes attributes0, PxFilterData filterData0, PxFilterObjectAttributes attributes1, PxFilterData filterData1, PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
{
	return PxFilterFlags();
}