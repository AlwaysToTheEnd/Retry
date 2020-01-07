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
class IPhysicsDevice;
class GameObject;

class CGHScene
{
	friend class GameObject;
	friend class DeviceObject;
public:
	CGHScene(IGraphicDevice* graphicDevice, IPhysicsDevice* pxDevice, const std::string& name);
	virtual ~CGHScene();
	
	virtual void		Init() = 0;
	virtual bool		Update(const DirectX::Mouse::ButtonStateTracker& mouse, float delta);
	void				DeleteAllObjects() { m_Objects.clear(); m_NumNullptr=0; }
	void				DeleteGameObject(GameObject* object);

	const std::string&	GetSceneName() const { return m_SceneName; }

protected:
	template<typename T, typename ...Types> T*	CreateGameObject(Types... args);
	void										AddGameObject(GameObject* object);

private:
	DeviceObjectUpdater& GetComponentUpdater(const char* typeName);
	void UnRegisterDeviceObject(const char* typeName, DeviceObject* gameObject);
	void RegisterDeviceObject(const char* typeName, DeviceObject* gameObject);


private:
	std::string												m_SceneName;
	IGraphicDevice*											m_GraphicDevice;
	IPhysicsDevice*											m_PhysicsDevice;
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
