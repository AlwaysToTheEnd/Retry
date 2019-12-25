#pragma once
#include "ComponentUpdater.h"
#include "GameObject.h"
#include <functional>
#include <DirectXMath.h>
#include <Mouse.h>

class IGraphicDevice;
class IPhysicsDevice;

class CGHScene
{
	friend class GameObject;
	friend class IComponent;
public:
	CGHScene(IGraphicDevice* graphicDevice, IPhysicsDevice* pxDevice, const std::string& name);
	virtual ~CGHScene();

	const void* Update(const DirectX::Mouse::ButtonStateTracker& mouse);
	void AddGameObjects(GameObject* newObject);
	void DeleteAllObjects() { m_Objects.clear(); }
	ComponentUpdater& GetComponentUpdater(COMPONENTTYPE type);
	const std::string& GetSceneName() const { return m_SceneName; }

private:
	const void* ExcuteFuncOfClickedObject(float dist);
	void ComponentDeleteManaging(COMPONENTTYPE type, int id);
	std::unique_ptr<IComponent> CreateComponent(COMPONENTTYPE type, GameObject& gameObject);

private:
	std::string									m_SceneName;

	ComponentUpdater							m_ComUpdater[IComponent::NUMCOMPONENTTYPE];
	std::vector<std::unique_ptr<GameObject>>	m_Objects;
	IGraphicDevice*								m_GraphicDevice;
	IPhysicsDevice*								m_PhysicsDevice;
};