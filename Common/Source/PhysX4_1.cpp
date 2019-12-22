#include "PhysX4_1.h"
#include "BaseComponent.h"
#include <Windows.h>
using namespace physx;


PhysX4_1::PhysX4_1()
{
}

PhysX4_1::~PhysX4_1()
{
	m_Material = nullptr;
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
	// 가장 기초가 되는 Foundation 생성, 싱글톤 클래스임에 주의하자.
	m_Foundation = PxCreateFoundation(PX_PHYSICS_VERSION,
		m_Allocator, m_ErrorCallback);

	// Physics Visual Debugger에 연결하기 포트 생성및 연결 작업. 
	// 사용하지 않는다면 하지 않아도 무방하다.

	m_PVD = PxCreatePvd(*m_Foundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
	m_PVD->connect(*transport, PxPvdInstrumentationFlag::eALL);

	// 모든 Physics 리소스 생성을 위한 메인 클래스 생성, 싱글톤 클래스임에 주의하자.
	// DirectX Device와 같은 개념으로 이해하면 좋다.
	// PxTolerancesScale 모든 물리처리에 있어서 기준의 되는 단위 설정.
	// length(1) 
	// mass(1000) 
	// speed(10) 

	m_Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_Foundation,
		PxTolerancesScale(),
		true, m_PVD.Get());
	PxRegisterHeightFields(*m_Physics);

	SYSTEM_INFO info;
	GetSystemInfo(&info);

	// CPU작업을 어떤식으로 할지 정한다. 씬에 적용
	// 쓰레드의 수를 정한다.
	m_Dispatcher = PxDefaultCpuDispatcherCreate(info.dwNumberOfProcessors);

	// 오브젝트들이 시뮬레이션 될 공간을 생성한다.
	// 씬 안에 시뮬레이션 될 액터 등 이 담기고 씬을 기준으로 시뮬레이션 한다.
	// PxSceneDesc는 씬의 속성 정보를 담은 구조체이다.
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

	// 씬에서의 시뮬레이션,위상 이벤트등을 콜백 인터페이스로 제어한다

	// 콜백규칙
	// 콜백은 메인스레드 또는 시뮬레이션 스레드에서 동시에 실행될 수 있으므로
	// SDK의 상태를 수정하면 안되며 특히 객체를 생성하거나 파괴해서는 안된다
	// 상태 수정이 필요한 경우 변경 내용을 버퍼에 저장 후 시뮬레이션 단계 이후 수행할 것
	//static SceneSimulationEventCallBack custumSimulationEvent;
	//sceneDesc.simulationEventCallback = &custumSimulationEvent;

	m_Scene = m_Physics->createScene(sceneDesc);

	//pvd 클라이언트 세팅
	PxPvdSceneClient* pvdClient = m_Scene->getScenePvdClient();
	if (pvdClient)
	{
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
		m_Scene->setVisualizationParameter(PxVisualizationParameter::eACTOR_AXES, 2.0f);
	}

	// 대량의 데이터를 생성하고 변환, 직렬화 하는 유틸리티 생성
	m_Cooking = PxCreateCooking(PX_PHYSICS_VERSION, *m_Foundation,
		PxCookingParams(PxTolerancesScale()));

	m_Material = m_Physics->createMaterial(0.5f, 0.5f, 0.6f);
	auto ground = PxCreatePlane(*m_Physics, PxPlane(0, 1, 0, 0), *m_Material);

	m_Scene->addActor(*ground);

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

	static PxTransform identityTransform(PxIDENTITY::PxIdentity);
	identityTransform.p.y = 4;
	PxRigidActor* rigidBody = nullptr;

	switch (type)
	{
	case COMPONENTTYPE::COM_DYNAMIC:
	{
		//test code
		PxShape* shape= m_Physics->createShape(PxBoxGeometry(3.0f, 3.0f, 3.0f), *m_Material, true);

		rigidBody = m_Physics->createRigidDynamic(identityTransform);
		rigidBody->attachShape(*shape);
		PxRigidBodyExt::updateMassAndInertia(*reinterpret_cast<PxRigidBody*>(rigidBody), 10.0f);
		
		m_Dynamics.AddData(rigidBody);
		newComponent = new ComRigidDynamic(gameObject, id, reinterpret_cast<PxRigidDynamic*>(rigidBody));

		//shape->release();
	}
	break;
	case COMPONENTTYPE::COM_STATIC:
	{
		rigidBody = m_Physics->createRigidStatic(identityTransform);
		m_Statics.AddData(rigidBody);
		newComponent = new ComRigidStatic(gameObject, id, reinterpret_cast<PxRigidStatic*>(rigidBody));
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

	if (rigidBody)
	{
		m_Scene->addActor(*rigidBody);
	}

	return std::unique_ptr<IComponent>(newComponent);
}

void PhysX4_1::ExcuteFuncOfClickedObject(float origin_x, float origin_y, float origin_z,
	float ray_x, float ray_y, float ray_z, float dist)
{
	if (m_Scene.Get())
	{
		PxVec3 origin(origin_x, origin_y, origin_z);
		PxVec3 ray(ray_x, ray_y, ray_z);
		PxRaycastBuffer rayBuffer;
		PxQueryFlags queryFlags= PxQueryFlag::eDYNAMIC;
		PxQueryFilterData filterData(PxFilterData(), queryFlags);

		m_Scene->raycast(origin, ray, dist, rayBuffer, PxHitFlags(0), filterData);
		
		PxU32 hitNum = rayBuffer.getNbAnyHits();
		PxU32 hitDistance = -1;
		PxRigidActor* targetActor = nullptr;
		for (PxU32 i = 0; i < hitNum; i++)
		{
			auto hitObject = rayBuffer.getAnyHit(i);
			
			/*PxVec3 pos = origin+(hitObject.distance*ray);
			PxVec3 pos2 = hitObject.actor->getGlobalPose().p;*/

			if (hitObject.distance < hitDistance)
			{
				targetActor = hitObject.actor;
				hitDistance = hitObject.distance;
			}
		}

		if (targetActor)
		{
			if (targetActor->userData)
			{
				auto functionlObject = reinterpret_cast<PhysXFunctionalObject*>(targetActor->userData);

				if (functionlObject->IsValideObject())
				{
					for (auto& it : functionlObject->voidFuncs)
					{
						it();
					}
				}
			}
		}
	}
}

void PhysX4_1::ComponentDeleteManaging(COMPONENTTYPE type, int id)
{
	auto& ComUpdater = GetComponentUpdater(type);
	IComponent* deletedComponent = ComUpdater.GetData(id);
	PxRigidActor* deletedActor = nullptr;

	switch (type)
	{
	case COMPONENTTYPE::COM_DYNAMIC:
	{
		deletedActor = reinterpret_cast<ComRigidDynamic*>(deletedComponent)->GetRigidBody();
		m_Dynamics.SignalDeleted(id);
	}
	break;
	case COMPONENTTYPE::COM_STATIC:
	{
		deletedActor = reinterpret_cast<ComRigidStatic*>(deletedComponent)->GetRigidBody();
		m_Statics.SignalDeleted(id);
	}
	break;
	case COMPONENTTYPE::COM_TRANSFORM:
		break;
	default:
		assert(false);
		break;
	}

	if (deletedActor)
	{
		if (deletedActor->userData)
		{
			auto functionlObject = reinterpret_cast<PhysXFunctionalObject*>(deletedActor->userData);

			if (functionlObject->IsValideObject())
			{
				delete functionlObject;
			}
		}

		m_Scene->removeActor(*deletedActor);
	}

	ComUpdater.SignalDeleted(id);
}

PxFilterFlags PhysX4_1::ScissorFilter(PxFilterObjectAttributes attributes0,
	PxFilterData filterData0, PxFilterObjectAttributes attributes1,
	PxFilterData filterData1, PxPairFlags& pairFlags,
	const void* constantBlock, PxU32 constantBlockSize)
{
	return PxFilterFlags();
}