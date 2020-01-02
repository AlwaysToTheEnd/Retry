#include "CGHScene.h"
#include "IGraphicDevice.h"
#include "IPhysicsDevice.h"
#include "d3dApp.h"
#include "GameObject.h"



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

bool CGHScene::Update(const DirectX::Mouse::ButtonStateTracker& mouse, unsigned long long delta)
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

	GetComponentUpdater(COMPONENTTYPE::COM_UICOLLISTION).Update(delta);
	physx::PxVec3 rayOrigin;
	physx::PxVec3 ray;
	GETAPP->GetMouseRay(rayOrigin, ray);

	if (mouse.leftButton == MOUSEState::UP)
	{
		m_PhysicsDevice->ExcuteFuncOfClickedObject(*this, rayOrigin.x, rayOrigin.y, rayOrigin.z,
			ray.x, ray.y, ray.z,500.0f, GameObject::CLICKEDSTATE::MOUSEOVER);
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
	GetComponentUpdater(COMPONENTTYPE::COM_STATIC).Update(delta);
	GetComponentUpdater(COMPONENTTYPE::COM_DYNAMIC).Update(delta);

	GetComponentUpdater(COMPONENTTYPE::COM_ANIMATOR).Update(delta);
	GetComponentUpdater(COMPONENTTYPE::COM_RENDERER).Update(delta);
	GetComponentUpdater(COMPONENTTYPE::COM_FONT).Update(delta);

	m_GraphicDevice->Update(*this);

	return result;
}

void CGHScene::ComponentDeleteManaging(COMPONENTTYPE type, int id)
{
	if (type == COMPONENTTYPE::COM_END)
	{
		assert(false && "THIS COMPONENT IS NONE USED TYPE");
	}

	auto& comUpdater = GetComponentUpdater(type);

	if (type > COMPONENTTYPE::COM_TRANSFORM)
	{
		m_GraphicDevice->ComponentDeleteManaging(*this, type, comUpdater.GetData(id));
	}
	else
	{
		m_PhysicsDevice->ComponentDeleteManaging(*this, type, comUpdater.GetData(id));
	}

	comUpdater.SignalDeleted(id);
}

std::unique_ptr<IComponent> CGHScene::CreateComponent(COMPONENTTYPE type, GameObject& gameObject)
{
	assert(type != COMPONENTTYPE::COM_END && "THIS COMPONENT IS NONE USED TYPE");

	IComponent* newComponent = nullptr;
	auto& comUpdater = GetComponentUpdater(type);
	UINT nextID = comUpdater.GetNextID();

	if (type > COMPONENTTYPE::COM_TRANSFORM)
	{
		newComponent = m_GraphicDevice->CreateComponent(*this, type, nextID, gameObject);
	}
	else
	{
		newComponent = m_PhysicsDevice->CreateComponent(*this, type, nextID, gameObject);
	}

	if (newComponent)
	{
		comUpdater.AddData(newComponent);
	}

	return std::unique_ptr<IComponent>(newComponent);
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

ComponentUpdater& CGHScene::GetComponentUpdater(COMPONENTTYPE type)
{
	unsigned int index = static_cast<unsigned int>(type);

	return m_ComUpdater[index];
}
