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
	// 가장 기초가 되는 Foundation 생성, 싱글톤 클래스임에 주의하자.
	m_foundation = PxCreateFoundation(PX_PHYSICS_VERSION,
		m_allocator, m_errorCallback);

	// Physics Visual Debugger에 연결하기 포트 생성및 연결 작업. 
	// 사용하지 않는다면 하지 않아도 무방하다.

	m_PVD = PxCreatePvd(*m_foundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
	m_PVD->connect(*transport, PxPvdInstrumentationFlag::eALL);

	// 모든 Physics 리소스 생성을 위한 메인 클래스 생성, 싱글톤 클래스임에 주의하자.
	// DirectX Device와 같은 개념으로 이해하면 좋다.
	// PxTolerancesScale 모든 물리처리에 있어서 기준의 되는 단위 설정.
	// length(1) 
	// mass(1000) 
	// speed(10) 

	m_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_foundation,
		PxTolerancesScale(),
		true, m_PVD.Get());
	PxRegisterHeightFields(*m_physics);

	SYSTEM_INFO info;
	GetSystemInfo(&info);

	// CPU작업을 어떤식으로 할지 정한다. 씬에 적용
	// 쓰레드의 수를 정한다.
	m_dispatcher = PxDefaultCpuDispatcherCreate(info.dwNumberOfProcessors);

	// 오브젝트들이 시뮬레이션 될 공간을 생성한다.
	// 씬 안에 시뮬레이션 될 액터 등 이 담기고 씬을 기준으로 시뮬레이션 한다.
	// PxSceneDesc는 씬의 속성 정보를 담은 구조체이다.
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

	// 씬에서의 시뮬레이션,위상 이벤트등을 콜백 인터페이스로 제어한다

	// 콜백규칙
	// 콜백은 메인스레드 또는 시뮬레이션 스레드에서 동시에 실행될 수 있으므로
	// SDK의 상태를 수정하면 안되며 특히 객체를 생성하거나 파괴해서는 안된다
	// 상태 수정이 필요한 경우 변경 내용을 버퍼에 저장 후 시뮬레이션 단계 이후 수행할 것
	//static SceneSimulationEventCallBack custumSimulationEvent;
	//sceneDesc.simulationEventCallback = &custumSimulationEvent;

	m_scene = m_physics->createScene(sceneDesc);

	//pvd 클라이언트 세팅
	PxPvdSceneClient* pvdClient = m_scene->getScenePvdClient();
	if (pvdClient)
	{
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
		m_scene->setVisualizationParameter(PxVisualizationParameter::eACTOR_AXES, 2.0f);
	}

	// 대량의 데이터를 생성하고 변환, 직렬화 하는 유틸리티 생성
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