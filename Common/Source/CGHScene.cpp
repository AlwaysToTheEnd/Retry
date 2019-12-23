#include "CGHScene.h"
#include "IGraphicDevice.h"
#include "IPhysicsDevice.h"
#include "d3dApp.h"

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

void CGHScene::AddGameObjects(GameObject* newObject)
{
	assert(newObject);

	newObject->Init();

	m_Objects.push_back(std::unique_ptr<GameObject>(newObject));
}

void CGHScene::Update()
{
	for (auto& it : m_Objects)
	{
		it->Update();
	}

	if (GETAPP->GetMouse().leftButton == MOUSEState::RELEASED)
	{
		GetComponentUpdater(COMPONENTTYPE::COM_UICOLLISTION).Update();
		ExcuteFuncOfClickedObject(500.0f);
	}

	m_PhysicsDevice->Update(*this);
	GetComponentUpdater(COMPONENTTYPE::COM_STATIC).Update();
	GetComponentUpdater(COMPONENTTYPE::COM_DYNAMIC).Update();

	GetComponentUpdater(COMPONENTTYPE::COM_ANIMATOR).Update();
	GetComponentUpdater(COMPONENTTYPE::COM_RENDERER).Update();
	GetComponentUpdater(COMPONENTTYPE::COM_FONT).Update();

	m_GraphicDevice->Update(*this);
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
		m_GraphicDevice->ComponentDeleteManaging(*this, type, id);
	}
	else
	{
		m_PhysicsDevice->ComponentDeleteManaging(*this, type, id);
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

ComponentUpdater& CGHScene::GetComponentUpdater(COMPONENTTYPE type)
{
	unsigned int index = static_cast<unsigned int>(type);

	return m_ComUpdater[index];
}

void CGHScene::ExcuteFuncOfClickedObject(float dist)
{
	DirectX::XMFLOAT3 rayOrigin;
	DirectX::XMFLOAT3 ray;

	GETAPP->GetMouseRay(rayOrigin, ray);
	m_PhysicsDevice->ExcuteFuncOfClickedObject(*this, rayOrigin.x, rayOrigin.y, rayOrigin.z,
		ray.x, ray.y, ray.z, dist);
}
