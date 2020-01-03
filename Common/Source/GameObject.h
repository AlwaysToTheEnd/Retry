#pragma once
#include <vector>
#include <unordered_map>
#include <memory>
#include <assert.h>
#include <functional>
#include <CGHScene.h>
#include "BaseClass.h"

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
	GameObject(CGHScene& scene);
	virtual ~GameObject() = default;

	CGHScene& GetScene() { return m_Scene; }

	template<typename T, typename ...Types> T* CreateGameObject(bool subordinate, Types... args);
	template<typename T> T* GetComponent();
	template<typename T> std::vector<T*> GetComponents();
	CLICKEDSTATE GetClickedState() { return m_State; }
	void SetClickedState(CLICKEDSTATE state) { m_State = state; }
	const GameObject* GetConstructor() const { return m_Constructor; }
	void SetConstructor(const GameObject* object) { m_Constructor = object; }
	void SetAllComponentActive(bool value);
	virtual void Delete();

	static std::unordered_map<unsigned int, unsigned int> GetComponentTypeIDs();
protected:
	virtual void Update(float delta) = 0;
	virtual void Init() = 0;

	template<typename T> T* AddComponent();
	template<typename T> void DeleteComponent(T* component = nullptr);

private:
	IComponent* CreateComponent(COMPONENTTYPE type);

private:
	static std::unordered_map<unsigned int, unsigned int>	m_TypeIDs;
	std::vector<std::unique_ptr<IComponent>>				m_Components[IComponent::NUMCOMPONENTTYPE];
	CGHScene&												m_Scene;
	const GameObject*										m_Constructor;
	CLICKEDSTATE											m_State;
};

template<typename T, typename ...Types>
inline T* GameObject::CreateGameObject(bool subordinate, Types ...args)
{
	T* result = new T(m_Scene, args...);

	if (subordinate)
	{
		result->SetConstructor(m_Constructor);
	}

	m_Scene.AddGameObject(result);

	return result;
}

template<typename T>
inline T* GameObject::GetComponent()
{
	T* component = nullptr;
	auto iter = m_TypeIDs.find(typeid(T).hash_code());

	if (iter != m_TypeIDs.end())
	{
		if (m_Components[iter->second].size())
		{
			component = reinterpret_cast<T*>(m_Components[iter->second].front().get());
		}
	}

	return component;
}

template<typename T>
inline std::vector<T*> GameObject::GetComponents()
{
	std::vector<T*> components;
	auto iter = m_TypeIDs.find(typeid(T).hash_code());

	if (iter != m_TypeIDs.end())
	{
		if (m_Components[iter->second].size())
		{
			for (auto& it : m_Components[iter->second])
			{
				components.push_back(reinterpret_cast<T*>(it.get()));
			}
		}
	}

	return components;
}

template<typename T>
inline T* GameObject::AddComponent()
{
	T* component = nullptr;
	auto iter = m_TypeIDs.find(typeid(T).hash_code());

	if (iter != m_TypeIDs.end())
	{
		component = static_cast<T*>(CreateComponent(static_cast<COMPONENTTYPE>(iter->second)));
	}

	return component;
}

template<typename T>
inline void GameObject::DeleteComponent(T* component)
{
	auto iter = m_TypeIDs.find(typeid(T).hash_code());

	if (iter != m_TypeIDs.end())
	{
		unsigned int index = iter->second;
		if (component == nullptr)
		{
			m_Components[index].pop_back();
		}
		else
		{
			for (auto comIter = m_Components[index].begin(); comIter != m_Components[index].end(); comIter++)
			{
				if (comIter->get() == component)
				{
					m_Components[index].erase(comIter);
					break;
				}
			}
		}
	}
}
