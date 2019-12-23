#include "PhysX4_1.h"
#include "BaseComponent.h"
#include "CGHScene.h"
#include "d3dApp.h"
#include <Windows.h>
using namespace physx;


PhysX4_1::PhysX4_1()
{
	m_GraphicsDevice = nullptr;
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
		sceneDesc.flags |= PxSceneFlag::eENABLE_GPU_DYNAMICS;
		sceneDesc.flags |= PxSceneFlag::eENABLE_PCM;
		sceneDesc.flags |= PxSceneFlag::eENABLE_STABILIZATION;
		sceneDesc.broadPhaseType = PxBroadPhaseType::eGPU;
		sceneDesc.gpuMaxNumPartitions = 8;
	}

	auto addedScene = m_Physics->createScene(sceneDesc);
	auto ground = PxCreatePlane(*m_Physics, PxPlane(0, 1, 0, 0), *m_Material);
	addedScene->addActor(*ground);

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

IComponent* PhysX4_1::CreateComponent(CGHScene& scene, COMPONENTTYPE type, unsigned int id, GameObject& gameObject)
{
	IComponent* newComponent = nullptr;

	static PxTransform identityTransform(PxIDENTITY::PxIdentity);
	identityTransform.p.y = 4;
	PxRigidActor* rigidBody = nullptr;

	switch (type)
	{
	case COMPONENTTYPE::COM_DYNAMIC:
	{
		//test code
		PxShape* shape = m_Physics->createShape(PxBoxGeometry(3.0f, 3.0f, 3.0f), *m_Material, true);

		rigidBody = m_Physics->createRigidDynamic(identityTransform);
		rigidBody->attachShape(*shape);
		PxRigidBodyExt::updateMassAndInertia(*reinterpret_cast<PxRigidBody*>(rigidBody), 10.0f);

		newComponent = new ComRigidDynamic(gameObject, id, reinterpret_cast<PxRigidDynamic*>(rigidBody));

		//shape->release();
	}
	break;
	case COMPONENTTYPE::COM_STATIC:
	{
		rigidBody = m_Physics->createRigidStatic(identityTransform);
		newComponent = new ComRigidStatic(gameObject, id, reinterpret_cast<PxRigidStatic*>(rigidBody));
	}
	break;
	case COMPONENTTYPE::COM_TRANSFORM:
		newComponent = new ComTransform(gameObject, id);
		break;
	case COMPONENTTYPE::COM_UICOLLISTION:
		newComponent = new ComUICollision(gameObject, id, &m_Scenes[scene.GetSceneName()].reservedToCheckUIs);
		break;
	default:
		assert(false);
		break;
	}

	if (rigidBody)
	{
		m_Scenes[scene.GetSceneName()].scene->addActor(*rigidBody);
	}

	return newComponent;
}

void PhysX4_1::ComponentDeleteManaging(CGHScene& scene, COMPONENTTYPE type, int id)
{
	IComponent* deletedComponent = scene.GetComponentUpdater(type).GetData(id);
	PxRigidActor* deletedActor = nullptr;

	switch (type)
	{
	case COMPONENTTYPE::COM_DYNAMIC:
	{
		deletedActor = reinterpret_cast<ComRigidDynamic*>(deletedComponent)->GetRigidBody();
	}
	break;
	case COMPONENTTYPE::COM_STATIC:
	{
		deletedActor = reinterpret_cast<ComRigidStatic*>(deletedComponent)->GetRigidBody();
	}
	break;
	case COMPONENTTYPE::COM_UICOLLISTION:
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

		m_Scenes[scene.GetSceneName()].scene->removeActor(*deletedActor);
	}
}

void PhysX4_1::ExcuteFuncOfClickedObject(CGHScene& scene, float origin_x, float origin_y, float origin_z,
	float ray_x, float ray_y, float ray_z, float dist)
{
	auto iter = m_Scenes.find(scene.GetSceneName());

	if (!CheckUIClicked(iter->second.reservedToCheckUIs))
	{
		auto currScene = iter->second.scene.Get();

		PxVec3 origin(origin_x, origin_y, origin_z);
		PxVec3 ray(ray_x, ray_y, ray_z);
		PxRaycastBuffer rayBuffer;
		PxQueryFlags queryFlags = PxQueryFlag::eDYNAMIC | PxQueryFlag::eSTATIC;
		PxQueryFilterData filterData(PxFilterData(), queryFlags);

		currScene->raycast(origin, ray, dist, rayBuffer, PxHitFlags(0), filterData);

		PxU32 hitNum = rayBuffer.getNbAnyHits();
		PxU32 hitDistance = -1;
		PxRigidActor* targetActor = nullptr;
		for (PxU32 i = 0; i < hitNum; i++)
		{
			auto hitObject = rayBuffer.getAnyHit(i);

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
					for (auto& it : functionlObject->m_VoidFuncs)
					{
						it();
					}
				}
			}
		}
	}
}

bool PhysX4_1::CheckUIClicked(std::vector<UICollisions>& collisions)
{
	PxVec3 vec3Pos[4];
	PxVec2 vec2Pos[4];
	PxMat44 mat;
	PxVec2 mousePos;
	DirectX::XMStoreFloat2(reinterpret_cast<DirectX::XMFLOAT2*>(&mousePos), GETAPP->GetMousePos());

	UICollisions* currUI = nullptr;
	float lastZ = 1000.0f;

	for (auto& it : collisions)
	{
		if (lastZ > it.transform.p.z)
		{
			mat = PxMat44(it.transform);
			vec3Pos[0] = mat.transform(PxVec3(-it.size.x, -it.size.y, it.transform.p.z));
			vec3Pos[1] = mat.transform(PxVec3(-it.size.x, it.size.y, it.transform.p.z));
			vec3Pos[2] = mat.transform(PxVec3(it.size.x, it.size.y, it.transform.p.z));
			vec3Pos[3] = mat.transform(PxVec3(it.size.x, -it.size.y, it.transform.p.z));


			for (int i = 0; i < 4; i++)
			{
				vec2Pos[i] = { vec3Pos[i].x, vec3Pos[i].y };
			}

			PxVec2 ltToMouseVec = mousePos - vec2Pos[0];
			PxVec2 ltToLbVec = vec2Pos[1] - vec2Pos[0];
			PxVec2 ltToRtVec = vec2Pos[3] - vec2Pos[0];

			PxVec2 rbToMouseVec = mousePos - vec2Pos[2];
			PxVec2 rbToLbVec = vec2Pos[1] - vec2Pos[2];
			PxVec2 rbToRtVec = vec2Pos[3] - vec2Pos[2];

			if (ltToLbVec.dot(ltToMouseVec) >= 0
				&& ltToRtVec.dot(ltToMouseVec) >= 0
				&& rbToLbVec.dot(rbToMouseVec) >= 0
				&& rbToRtVec.dot(rbToMouseVec) >= 0)
			{
				lastZ = it.transform.p.z;
				currUI = &it;
			}
		}
	}

	if (currUI)
	{
		for (auto& it : currUI->voidFuncs)
		{
			it();
		}
	}

	collisions.clear();

	return currUI != nullptr;
}

PxFilterFlags PhysX4_1::ScissorFilter(PxFilterObjectAttributes attributes0,
	PxFilterData filterData0, PxFilterObjectAttributes attributes1,
	PxFilterData filterData1, PxPairFlags& pairFlags,
	const void* constantBlock, PxU32 constantBlockSize)
{
	return PxFilterFlags();
}