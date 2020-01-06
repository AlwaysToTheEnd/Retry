#pragma once
#include "ComponentUpdater.h"
#include "IComponent.h"
#include <vector>
#include <unordered_map>
#include <memory>
#include <assert.h>
#include <functional>
#include <DirectXMath.h>
#include <Mouse.h>

class IGraphicDevice;
class IPhysicsDevice;
class GameObject;

class CGHScene
{
	friend class GameObject;
	friend class IComponent;
public:
	CGHScene(IGraphicDevice* graphicDevice, IPhysicsDevice* pxDevice, const std::string& name);
	virtual ~CGHScene();
	
	virtual void Init() = 0;
	virtual bool Update(const DirectX::Mouse::ButtonStateTracker& mouse, float delta);
	void DeleteAllObjects() { m_Objects.clear(); m_NumNullptr=0; }
	void DeleteGameObject(GameObject* object);
	const std::string& GetSceneName() const { return m_SceneName; }

protected:
	template<typename T, typename ...Types> T* CreateGameObject(Types... args);
	void AddGameObject(GameObject* object);

private:
	ComponentUpdater& GetComponentUpdater(COMPONENTTYPE type);
	void ComponentDeleteManaging(COMPONENTTYPE type, int id);
	std::unique_ptr<IComponent> CreateComponent(COMPONENTTYPE type, GameObject& gameObject);

private:
	ComponentUpdater							m_ComUpdater[IComponent::NUMCOMPONENTTYPE];
	IGraphicDevice*								m_GraphicDevice;
	IPhysicsDevice*								m_PhysicsDevice;

protected:
	std::string									m_SceneName;
	std::vector<std::unique_ptr<GameObject>>	m_Objects;
	size_t										m_NumNullptr;
};

template<typename T, typename ...Types>
inline T* CGHScene::CreateGameObject(Types... args)
{
	T* newObject = new T(*this, args...);
	AddGameObject(newObject);

	return newObject;
}
