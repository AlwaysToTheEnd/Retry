#include "CGHScene.h"
#include "IGraphicDevice.h"
#include "PhysX4_1.h"
#include "d3dApp.h"
#include "GameObject.h"
#include "PhysicsDO.h"
#include "GraphicDO.h"
#include "SoundDO.h"


CGHScene::CGHScene(IGraphicDevice* graphicDevice, ISoundDevice* soundDevice, PhysX4_1* pxDevice, const std::string& name)
	:m_GraphicDevice(graphicDevice)
	, m_PhysicsDevice(pxDevice)
	, m_SoundDevice(soundDevice)
	, m_SceneName(name)
	, m_NumNullptr(0)
{
	m_GraphicDevice->CreateScene(*this);
	m_PhysicsDevice->CreateScene(*this);
}

CGHScene::~CGHScene()
{

}

void CGHScene::Update(const DirectX::Mouse::ButtonStateTracker& mouse, float delta)
{
	size_t numObjects = m_Objects.size() - m_NumNullptr;
	for (size_t i = 0; i < numObjects; i++)
	{
		m_Objects[i]->Update(delta);
	}

	m_PhysicsDevice->Update(*this);
	GetComponentUpdater(typeid(DORigidStatic).name()).Update(delta);
	GetComponentUpdater(typeid(DORigidDynamic).name()).Update(delta);

	GetComponentUpdater(typeid(DOAnimator).name()).Update(delta);
	GetComponentUpdater(typeid(DORenderer).name()).Update(delta);
	GetComponentUpdater(typeid(DOFont).name()).Update(delta);
	GetComponentUpdater(typeid(DOSound).name()).Update(delta);

	m_GraphicDevice->Update(*this, delta);
}

void CGHScene::PixelFuncDo(int id, const DirectX::Mouse::ButtonStateTracker& mouseTracker)
{
	if (id > -1)
	{
		auto renderer = GetComponentUpdater(typeid(DORenderer).name()).GetData(id)->Get<DORenderer>();
		renderer->DoFuncFromMouse(mouseTracker.leftButton, DirectX::MOUSEBUTTONINDEX::LEFTBUTTON);
		renderer->DoFuncFromMouse(mouseTracker.middleButton, DirectX::MOUSEBUTTONINDEX::MIDDLEBUTTON);
		renderer->DoFuncFromMouse(mouseTracker.rightButton, DirectX::MOUSEBUTTONINDEX::RIGHTBUTTON);
	}
	else if (id < -1)
	{
		auto renderer = GetComponentUpdater(typeid(DOFont).name()).GetData(FONTRENDERERID(id))->Get<DOFont>();
		renderer->DoFuncFromMouse(mouseTracker.leftButton, DirectX::MOUSEBUTTONINDEX::LEFTBUTTON);
		renderer->DoFuncFromMouse(mouseTracker.middleButton, DirectX::MOUSEBUTTONINDEX::MIDDLEBUTTON);
		renderer->DoFuncFromMouse(mouseTracker.rightButton, DirectX::MOUSEBUTTONINDEX::RIGHTBUTTON);
	}
}

DeviceObjectUpdater& CGHScene::GetComponentUpdater(const char* typeName)
{
	return m_ComUpdater[typeName];
}

void CGHScene::UnRegisterDeviceObject(DeviceObject* gameObject)
{
	assert(gameObject);
	assert(gameObject->GetDeviceOBID() != -1);

	auto& comUpdater = GetComponentUpdater(gameObject->GetTypeName());

	m_GraphicDevice->UnRegisterDeviceObject(*this, gameObject);
	m_PhysicsDevice->UnRegisterDeviceObject(*this, gameObject);

	comUpdater.SignalDeleted(gameObject->GetDeviceOBID());
}

void CGHScene::RegisterDeviceObject(DeviceObject* gameObject)
{
	assert(gameObject);
	assert(gameObject->GetDeviceOBID() == -1);

	auto& comUpdater = GetComponentUpdater(gameObject->GetTypeName());
	gameObject->SetDeviceOBID(comUpdater.GetNextID());

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

	if (object->IsObjectType(GameObject::GAMEOBJECT_TYPE::DEVICE_OBJECT))
	{
		auto deviceOB = reinterpret_cast<DeviceObject*>(object);
		RegisterDeviceObject(deviceOB);

		deviceOB->Init(m_GraphicDevice, m_SoundDevice, m_PhysicsDevice);
	}
	else
	{
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
}
