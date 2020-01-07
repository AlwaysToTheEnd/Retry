#include "CGHScene.h"
#include "IGraphicDevice.h"
#include "IPhysicsDevice.h"
#include "d3dApp.h"
#include "GameObject.h"
#include "PhysicsDO.h"
#include "GraphicDO.h"


CGHScene::CGHScene(IGraphicDevice* graphicDevice, IPhysicsDevice* pxDevice, const std::string& name)
	:m_GraphicDevice(graphicDevice)
	, m_PhysicsDevice(pxDevice)
	, m_SceneName(name)
	, m_NumNullptr(0)
{
	m_GraphicDevice->CreateScene(*this);
	m_PhysicsDevice->CreateScene(*this);
}

CGHScene::~CGHScene()
{

}

bool CGHScene::Update(const DirectX::Mouse::ButtonStateTracker& mouse, float delta)
{
	bool result = true;

	size_t numObjects = m_Objects.size() - m_NumNullptr;
	for (size_t i = 0; i < numObjects; i++)
	{
		m_Objects[i]->Update(delta);
	}

	for (size_t i = 0; i < numObjects; i++)
	{
		m_Objects[i]->SetClickedState(GameObject::CLICKEDSTATE::NONE);
	}

	GetComponentUpdater(typeid(DOUICollision).name()).Update(delta);
	physx::PxVec3 rayOrigin;
	physx::PxVec3 ray;
	GETAPP->GetMouseRay(rayOrigin, ray);

	if (mouse.leftButton == MOUSEState::UP)
	{
		m_PhysicsDevice->ExcuteFuncOfClickedObject(*this, rayOrigin.x, rayOrigin.y, rayOrigin.z,
			ray.x, ray.y, ray.z, 500.0f, GameObject::CLICKEDSTATE::MOUSEOVER);
	}
	else if (mouse.leftButton == MOUSEState::PRESSED)
	{
		result = m_PhysicsDevice->ExcuteFuncOfClickedObject(*this, rayOrigin.x, rayOrigin.y, rayOrigin.z,
			ray.x, ray.y, ray.z, 500.0f, GameObject::CLICKEDSTATE::PRESSED);
	}
	else if (mouse.leftButton == MOUSEState::HELD)
	{
		m_PhysicsDevice->ExcuteFuncOfClickedObject(*this, rayOrigin.x, rayOrigin.y, rayOrigin.z,
			ray.x, ray.y, ray.z, 500.0f, GameObject::CLICKEDSTATE::HELD);
	}
	else if (mouse.leftButton == MOUSEState::RELEASED)
	{
		result = m_PhysicsDevice->ExcuteFuncOfClickedObject(*this, rayOrigin.x, rayOrigin.y, rayOrigin.z,
			ray.x, ray.y, ray.z, 500.0f, GameObject::CLICKEDSTATE::RELEASED);
	}


	m_PhysicsDevice->Update(*this);
	GetComponentUpdater(typeid(DORigidStatic).name()).Update(delta);
	GetComponentUpdater(typeid(DORigidDynamic).name()).Update(delta);

	GetComponentUpdater(typeid(DOAnimator).name()).Update(delta);
	GetComponentUpdater(typeid(DORenderer).name()).Update(delta);
	GetComponentUpdater(typeid(DOFont).name()).Update(delta);

	m_GraphicDevice->Update(*this);

	return result;
}

DeviceObjectUpdater& CGHScene::GetComponentUpdater(const char* typeName)
{
	return m_ComUpdater[typeName];
}

void CGHScene::UnRegisterDeviceObject(const char* typeName, DeviceObject* gameObject)
{
	assert(gameObject);
	assert(gameObject->GetID() != -1);

	auto& comUpdater = GetComponentUpdater(typeName);

	m_GraphicDevice->UnRegisterDeviceObject(*this, gameObject);
	m_PhysicsDevice->UnRegisterDeviceObject(*this, gameObject);

	comUpdater.SignalDeleted(gameObject->GetID());
}

void CGHScene::RegisterDeviceObject(const char* typeName, DeviceObject* gameObject)
{
	assert(gameObject);
	assert(gameObject->GetID() == -1);

	auto& comUpdater = GetComponentUpdater(typeName);
	gameObject->SetID(comUpdater.GetNextID());

	m_GraphicDevice->RegisterDeviceObject(*this, gameObject);
	m_PhysicsDevice->RegisterDeviceObject(*this, gameObject);

	comUpdater.AddData(gameObject);
}

void CGHScene::DeleteGameObject(GameObject* object)
{
	if (object)
	{
		size_t numObjects = m_Objects.size() - m_NumNullptr;

		for (size_t i = 0; i < numObjects; i++)
		{
			if (m_Objects[i].get() == object)
			{
				m_NumNullptr++;
				m_Objects[i] = nullptr;
				m_Objects[i] = std::move(m_Objects[numObjects - 1]);
				break;
			}
		}
	}
}

void CGHScene::AddGameObject(GameObject* object)
{
	assert(object);

	if (m_NumNullptr)
	{
		m_Objects[m_Objects.size() - m_NumNullptr] = std::unique_ptr<GameObject>(object);
		m_NumNullptr--;
	}
	else
	{
		m_Objects.push_back(std::unique_ptr<GameObject>(object));
	}

	object->Init();
}
