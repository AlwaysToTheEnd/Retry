#pragma once
#include "PxPhysicsAPI.h"
#include "IPhysicsDevice.h"
#include "PhysXFunctionalObject.h"
#include "Px1RefPtr.h"


struct PhysxSceneObject
{
	Px1RefPtr<physx::PxScene>	scene;
	std::vector<UICollisions>	reservedToCheckUIs;
};

class PhysX4_1 final : public IPhysicsDevice
{
	class SceneSimulationFilterCallBack :public physx::PxSimulationFilterCallback
	{
	public:
		virtual ~SceneSimulationFilterCallBack() {}

		virtual	physx::PxFilterFlags pairFound(physx::PxU32 pairID,
			physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0, const physx::PxActor* a0, const physx::PxShape* s0,
			physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1, const physx::PxActor* a1, const physx::PxShape* s1,
			physx::PxPairFlags& pairFlags)
		{
			if (a0->userData)
			{
				m_Objects.push_back(reinterpret_cast<PhysXFunctionalObject*>(a0->userData));
			}

			if (a1->userData)
			{
				m_Objects.push_back(reinterpret_cast<PhysXFunctionalObject*>(a1->userData));
			}

			return physx::PxFilterFlag::eCALLBACK;
		}

		virtual	void pairLost(physx::PxU32 pairID, physx::PxFilterObjectAttributes attributes0, 
			physx::PxFilterData filterData0, physx::PxFilterObjectAttributes attributes1, 
			physx::PxFilterData filterData1, bool objectRemoved)
		{

		}

		virtual bool statusChange(physx::PxU32& pairID, physx::PxPairFlags& pairFlags, 
			physx::PxFilterFlags& filterFlags)
		{
			return true;
		}

		void ReservedObjectsExcute()
		{
			for (auto& it : m_Objects)
			{
				if (it->IsValideObject())
				{
					for (auto& it2 : it->m_VoidFuncs)
					{
						it2();
					}
				}
			}

			m_Objects.clear();
		}

	private:
		std::vector<PhysXFunctionalObject*> m_Objects;

	} m_SceneSimulationFilterCallBack;

public:
	PhysX4_1();
	virtual ~PhysX4_1();

	virtual bool Init(void* graphicDevicePtr);
	virtual void Update(const CGHScene& scene);
	virtual void CreateScene(const CGHScene& scene) override;

	virtual IComponent* CreateComponent(CGHScene& scene, COMPONENTTYPE type, unsigned int id, GameObject& gameObject) override;
	virtual void ComponentDeleteManaging(CGHScene& scene, COMPONENTTYPE type, IComponent* deletedCom) override;
	virtual bool ExcuteFuncOfClickedObject(CGHScene& scene, float origin_x, float origin_y, float origin_z,
		float ray_x, float ray_y, float ray_z, float dist, bool isExcute=true) override;

private:
	bool CheckUIClicked(std::vector<UICollisions>& collisions, bool isExcute);

private:
	physx::PxFilterFlags ScissorFilter(physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0,
		physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1,
		physx::PxPairFlags& pairFlags, const void* constantBlock, physx::PxU32 constantBlockSize);

private:
	physx::PxDefaultAllocator									m_Allocator;
	physx::PxDefaultErrorCallback								m_ErrorCallback;
	void*														m_GraphicsDevice;
	
	Px1RefPtr<physx::PxFoundation>								m_Foundation;
	Px1RefPtr<physx::PxPhysics>									m_Physics;
	Px1RefPtr<physx::PxDefaultCpuDispatcher>					m_Dispatcher;
	Px1RefPtr<physx::PxCooking>									m_Cooking;
	Px1RefPtr<physx::PxCudaContextManager>						m_CudaManager;
	Px1RefPtr<physx::PxPvd>										m_PVD;
	std::unordered_map<std::string, PhysxSceneObject>			m_Scenes;

private:
	Px1RefPtr<physx::PxMaterial>					m_Material;
	std::vector<PhysXFunctionalObject*>				m_ReservedFunc;
};

