#pragma once
#include <vector>
#include <unordered_map>
#include <memory>
#include <assert.h>
#include <functional>
#include <DirectXMath.h>
#include <Mouse.h>
#include "ComponentUpdater.h"
#include "DeviceObject.h"

class IGraphicDevice;
class PhysX4_1;
class GameObject;

#define FONTRENDERERID(T) -(T+2)

class CGHScene
{
	friend class GameObject;
	friend class DeviceObject;
	const float m_RayDistance = 1000.0f;
public:
	CGHScene(IGraphicDevice* graphicDevice, PhysX4_1* pxDevice, const std::string& name);
	virtual ~CGHScene();
	
	virtual void		Init() = 0;
	virtual void		Update(const DirectX::Mouse::ButtonStateTracker& mouse, float delta);
	virtual void		PixelFuncDo(int id, const DirectX::Mouse::ButtonStateTracker& mouseTracker);
	void				DeleteAllObjects() { m_Objects.clear(); m_NumNullptr=0; }
	void				DeleteGameObject(GameObject* object);

	const std::string&	GetSceneName() const { return m_SceneName; }

	template<typename T, typename ...Types> T*	CreateGameObject(Types... args);
	void										AddGameObject(GameObject* object);

private:
	DeviceObjectUpdater& GetComponentUpdater(const char* typeName);
	void RegisterDeviceObject(DeviceObject* gameObject);
	void UnRegisterDeviceObject(DeviceObject* gameObject);

private:
	std::string												m_SceneName;
	IGraphicDevice*											m_GraphicDevice;
	PhysX4_1*												m_PhysicsDevice;
	std::vector<std::unique_ptr<GameObject>>				m_Objects;
	size_t													m_NumNullptr;
	std::unordered_map<std::string, DeviceObjectUpdater>	m_ComUpdater;
};

template<typename T, typename ...Types>
inline T* CGHScene::CreateGameObject(Types... args)
{
	T* newObject = new T(*this, nullptr, typeid(T).name(), args...);
	AddGameObject(newObject);

	return newObject;
}
