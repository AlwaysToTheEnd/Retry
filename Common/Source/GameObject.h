#pragma once
#include <vector>
#include <assert.h>
#include <functional>
#include "BaseClass.h"

class CGHScene;

class GameObject
{
public:
	enum class CLICKEDSTATE
	{
		NONE,
		MOUSEOVER,
		PRESSED,
		HELD,
		RELEASED,
	};

	friend class CGHScene;
public:
	GameObject(CGHScene& scene, GameObject* const parent, unsigned int hashCode, bool enrollment = true);
	virtual ~GameObject() = default;

	virtual void	Delete();
	void			ExceptComponent(GameObject* com);

	template<typename T, typename ...Types> T*	CreateComponenet(Types... args);
	void										SetClickedState(CLICKEDSTATE state) { m_State = state; }
	void										SetAllComponentActive(bool value);
	void										SetActive(bool value) { m_IsActive = value; }

	bool										Is(unsigned int hashCode) const { return m_HashCode == hashCode; }
	CLICKEDSTATE								GetClickedState() const { return m_State; }
	bool										GetActive() const { return m_IsActive; }
	template<typename T> T*						GetComponent();
	template<typename T> std::vector<T*>		GetComponents();
	CGHScene&									GetScene() { return m_Scene; }

protected:
	virtual void Update(float delta) = 0;
	virtual void Init() = 0;

protected:
	CGHScene&												m_Scene;
	GameObject* const										m_Parent;
	const unsigned int										m_HashCode;

	bool													m_IsActive;
	CLICKEDSTATE											m_State;
	std::vector<GameObject*>								m_Components;
};

template<typename T, typename ...Types>
inline T* GameObject::CreateComponenet(Types ...args)
{
	T* result = new T(m_Scene, this, typeid(T).hash_code(), args...);

	return result;
}

template<typename T>
inline T* GameObject::GetComponent()
{
	T* component = nullptr;

	for (auto& it : m_Components)
	{
		if (it->Is(typeid(T).hash_code())
		{
			component = static_cast<T*>(it);
		}
	}

	return component;
}

template<typename T>
inline std::vector<T*> GameObject::GetComponents()
{
	std::vector<T*> components;

	for (auto& it : m_Components)
	{
		if (it->Is(typeid(T).hash_code())
		{
			components.push_back(static_cast<T*>(it));
		}
	}

	return components;
}