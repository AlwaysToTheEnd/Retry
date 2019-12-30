#include "CGHScene.h"
#include "IGraphicDevice.h"
#include "IPhysicsDevice.h"
#include "d3dApp.h"
#include "GameObject.h"



CGHScene::CGHScene(IGraphicDevice* graphicDevice, IPhysicsDevice* pxDevice, const std::string& name)
	:m_GraphicDevice(graphicDevice)
	, m_PhysicsDevice(pxDevice)
	, m_SceneName(name)
{
	m_GraphicDevice->CreateScene(*this);
	m_PhysicsDevice->CreateScene(*this);
}

CGHScene::~CGHScene()
{

}

bool CGHScene::Update(const DirectX::Mouse::ButtonStateTracker& mouse)
{
	bool result = true;

	for (auto& it : m_Objects)
	{
		it->Update();
	}
	
	if (mouse.leftButton == MOUSEState::PRESSED)
	{
		GetComponentUpdater(COMPONENTTYPE::COM_UICOLLISTION).Update();

		DirectX::XMFLOAT3 rayOrigin;
		DirectX::XMFLOAT3 ray;

		GETAPP->GetMouseRay(rayOrigin, ray);
		result = m_PhysicsDevice->ExcuteFuncOfClickedObject(*this, rayOrigin.x, rayOrigin.y, rayOrigin.z,
			ray.x, ray.y, ray.z, 500.0f, false);
	}
	else if(mouse.leftButton == MOUSEState::RELEASED)
	{
		GetComponentUpdater(COMPONENTTYPE::COM_UICOLLISTION).Update();

		DirectX::XMFLOAT3 rayOrigin;
		DirectX::XMFLOAT3 ray;

		GETAPP->GetMouseRay(rayOrigin, ray);
		result = m_PhysicsDevice->ExcuteFuncOfClickedObject(*this, rayOrigin.x, rayOrigin.y, rayOrigin.z,
			ray.x, ray.y, ray.z, 500.0f);
	}
	
	m_PhysicsDevice->Update(*this);
	GetComponentUpdater(COMPONENTTYPE::COM_STATIC).Update();
	GetComponentUpdater(COMPONENTTYPE::COM_DYNAMIC).Update();

	GetComponentUpdater(COMPONENTTYPE::COM_ANIMATOR).Update();
	GetComponentUpdater(COMPONENTTYPE::COM_RENDERER).Update();
	GetComponentUpdater(COMPONENTTYPE::COM_FONT).Update();

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
	for (auto iter = m_Objects.begin(); iter != m_Objects.end(); iter++)
	{
		if (iter->get() == object)
		{
			m_Objects.erase(iter);
			break;
		}
	}
}

void CGHScene::AddGameObject(GameObject* object)
{
	assert(object);

	m_Objects.push_back(std::unique_ptr<GameObject>(object));
	m_Objects.back()->Init();
}

ComponentUpdater& CGHScene::GetComponentUpdater(COMPONENTTYPE type)
{
	unsigned int index = static_cast<unsigned int>(type);

	return m_ComUpdater[index];
}
