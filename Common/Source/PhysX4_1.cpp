#include "PhysX4_1.h"
#include "BaseComponent.h"
#include <Windows.h>
using namespace physx;


PhysX4_1::PhysX4_1()
{
}

PhysX4_1::~PhysX4_1()
{
	m_PlaneMaterial = nullptr;
	m_Scene = nullptr;
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
	// ���� ���ʰ� �Ǵ� Foundation ����, �̱��� Ŭ�����ӿ� ��������.
	m_Foundation = PxCreateFoundation(PX_PHYSICS_VERSION,
		m_Allocator, m_ErrorCallback);

	// Physics Visual Debugger�� �����ϱ� ��Ʈ ������ ���� �۾�. 
	// ������� �ʴ´ٸ� ���� �ʾƵ� �����ϴ�.

	m_PVD = PxCreatePvd(*m_Foundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
	m_PVD->connect(*transport, PxPvdInstrumentationFlag::eALL);

	// ��� Physics ���ҽ� ������ ���� ���� Ŭ���� ����, �̱��� Ŭ�����ӿ� ��������.
	// DirectX Device�� ���� �������� �����ϸ� ����.
	// PxTolerancesScale ��� ����ó���� �־ ������ �Ǵ� ���� ����.
	// length(1) 
	// mass(1000) 
	// speed(10) 

	m_Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_Foundation,
		PxTolerancesScale(),
		true, m_PVD.Get());
	PxRegisterHeightFields(*m_Physics);

	SYSTEM_INFO info;
	GetSystemInfo(&info);

	// CPU�۾��� ������� ���� ���Ѵ�. ���� ����
	// �������� ���� ���Ѵ�.
	m_Dispatcher = PxDefaultCpuDispatcherCreate(info.dwNumberOfProcessors);

	// ������Ʈ���� �ùķ��̼� �� ������ �����Ѵ�.
	// �� �ȿ� �ùķ��̼� �� ���� �� �� ���� ���� �������� �ùķ��̼� �Ѵ�.
	// PxSceneDesc�� ���� �Ӽ� ������ ���� ����ü�̴�.
	PxSceneDesc sceneDesc(m_Physics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	sceneDesc.cpuDispatcher = m_Dispatcher.Get();
	sceneDesc.broadPhaseType = PxBroadPhaseType::eABP;
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;
	
	if (graphicDevicePtr != nullptr)
	{
		PxCudaContextManagerDesc cudaDesc;
		cudaDesc.graphicsDevice = graphicDevicePtr;
		m_CudaManager = PxCreateCudaContextManager(*m_Foundation, cudaDesc);
		sceneDesc.gpuDispatcher = m_CudaManager->getGpuDispatcher();
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

	m_Scene = m_Physics->createScene(sceneDesc);

	//pvd Ŭ���̾�Ʈ ����
	PxPvdSceneClient* pvdClient = m_Scene->getScenePvdClient();
	if (pvdClient)
	{
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
		m_Scene->setVisualizationParameter(PxVisualizationParameter::eACTOR_AXES, 2.0f);
	}

	// �뷮�� �����͸� �����ϰ� ��ȯ, ����ȭ �ϴ� ��ƿ��Ƽ ����
	m_Cooking = PxCreateCooking(PX_PHYSICS_VERSION, *m_Foundation,
		PxCookingParams(PxTolerancesScale()));
	
	return true;
}

void PhysX4_1::Update()
{
	m_Scene->simulate(1.0f / 60.0f);
	m_Scene->fetchResults(true);
}

std::unique_ptr<IComponent> PhysX4_1::CreateComponent(COMPONENTTYPE type, GameObject& gameObject)
{
	IComponent* newComponent = nullptr;

	auto& ComUpdater = GetComponentUpdater(type);
	UINT id = ComUpdater.GetNextID();
	
	static PxTransform identityTransform = {};
	
	switch (type)
	{
	case COMPONENTTYPE::COM_DYNAMIC:
	{
		PxRigidDynamic* rigidBody = m_Physics->createRigidDynamic(identityTransform);
		m_Dynamics.AddData();
		m_Dynamics.GetData(id) = rigidBody;
		
		newComponent = new ComRigidDynamic(gameObject, id, rigidBody);
	}
		break;
	case COMPONENTTYPE::COM_STATIC:
	{
		PxRigidStatic* rigidBody = m_Physics->createRigidStatic(identityTransform);
		m_Statics.AddData();
		m_Statics.GetData(id) = rigidBody;
		newComponent = new ComRigidStatic(gameObject, id, rigidBody);
	}
		break;
	case COMPONENTTYPE::COM_TRANSFORM:
		newComponent = new ComTransform(gameObject, id);
		break;
	default:
		assert(false);
		break;
	}

	if (newComponent)
	{
		ComUpdater.AddData(newComponent);
	}

	return std::unique_ptr<IComponent>(newComponent);
}

void PhysX4_1::ComponentDeleteManaging(COMPONENTTYPE type, int id)
{
	auto& ComUpdater = GetComponentUpdater(type);
	IComponent* deletedComponent = ComUpdater.GetData(id);

	switch (type)
	{
	case COMPONENTTYPE::COM_DYNAMIC:
		m_Scene->removeActor(*reinterpret_cast<ComRigidDynamic*>(deletedComponent)->GetRigidBody());
		m_Dynamics.GetData(id) = nullptr;
		m_Dynamics.SignalDelete(id);
		break;
	case COMPONENTTYPE::COM_STATIC:
		m_Scene->removeActor(*reinterpret_cast<ComRigidStatic*>(deletedComponent)->GetRigidBody());
		m_Statics.GetData(id) = nullptr;
		m_Statics.SignalDelete(id);
	case COMPONENTTYPE::COM_TRANSFORM:
		break;
	default:
		assert(false);
		break;
	}

	ComUpdater.SignalDelete(id);
}

PxFilterFlags PhysX4_1::ScissorFilter(PxFilterObjectAttributes attributes0,
	PxFilterData filterData0, PxFilterObjectAttributes attributes1, 
	PxFilterData filterData1, PxPairFlags& pairFlags, 
	const void* constantBlock, PxU32 constantBlockSize)
{
	return PxFilterFlags();
}