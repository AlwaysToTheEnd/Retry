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

	enum GAMEOBJECT_TYPE
	{
		DEVICE_OBJECT = 1,
		PHYSICS_OBJECT= 1<<1,
		GRAPHIC_OBJECT= 1<<2,
	};

	friend class CGHScene;
public:
	GameObject(CGHScene& scene, GameObject* parent, const char* typeName);
	virtual ~GameObject() = default;

	virtual void	Delete();
	void			ExceptComponent(GameObject* com);

	template<typename T, typename ...Types> T*	CreateComponenet(bool dependent, Types... args);
	template<typename T> T*						CreateComponenet();
	void										SetClickedState(CLICKEDSTATE state) { m_State = state; }
	void										SetParent(GameObject* parent);
	virtual void								SetActive(bool value, bool components = false);

	template<typename T> bool					Is() const { return m_TypeName == typeid(T).name(); }
	template<typename T> T*						Get() { assert(Is<T>()); return static_cast<T*>(this); }
	virtual bool								IsObjectType(GAMEOBJECT_TYPE type) const { return false; }
	bool										GetActive() const { return m_IsActive; }
	const char*									GetTypeName() const { return m_TypeName; }
	CLICKEDSTATE								GetClickedState() const { return m_State; }
	const GameObject* const						GetConstructor() const;
	template<typename T> T*						GetComponent();
	template<typename T> std::vector<T*>		GetComponents();
	CGHScene&									GetScene() { return m_Scene; }

protected:
	virtual void Update(float delta) = 0;
	virtual void Init() = 0;

private:
	void InitObject(GameObject* object);

protected:
	CGHScene&					m_Scene;
	GameObject*					m_Parent;
	const char*					m_TypeName;

	bool						m_IsActive;
	CLICKEDSTATE				m_State;
	std::vector<GameObject*>	m_Components;
};

template<typename T, typename ...Types>
inline T* GameObject::CreateComponenet(bool dependent, Types ...args)
{
	T* result = nullptr;
	
	if (dependent)
	{
		result= new T(m_Scene, this, typeid(T).name(), args...);
		m_Components.push_back(result);
	}
	else
	{
		result = new T(m_Scene, nullptr, typeid(T).name(), args...);
	}

	InitObject(result);

	return result;
}

template<typename T>
inline T* GameObject::CreateComponenet()
{
	T* result = new T(m_Scene, this, typeid(T).name());

	m_Components.push_back(result);
	InitObject(result);

	return result;
}

template<typename T>
inline T* GameObject::GetComponent()
{
	T* component = nullptr;

	for (auto& it : m_Components)
	{
		if (it->Is<T>())
		{
			component = static_cast<T*>(it);
			break;
		}
	}

	return component;
}

template<typename T>
inline std::vector<T*> GameObject::GetComponents()
{
	std::vector<T*> result;

	for (size_t i = 0; i < m_Components.size(); i++)
	{
		if (m_Components[i]->Is<T>())
		{
			result.push_back(static_cast<T*>(m_Components[i]));
		}
	}
	
	return result;
}