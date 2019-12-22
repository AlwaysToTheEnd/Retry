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