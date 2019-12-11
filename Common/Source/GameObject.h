#pragma once
#include <vector>
#include <unordered_map>
#include <memory>
#include <assert.h>
#include "IComponent.h"

class GameObject
{
public:
	GameObject() = default;
	virtual ~GameObject() = default;

	virtual void Init() = 0;
	virtual void Update() = 0;

	template<typename T> T* GetComponent();
	template<typename T> std::vector<T*> GetComponents();

	static std::unordered_map<unsigned int, unsigned int> GetComponentTypeIDs();
protected:
	template<typename T> T* AddComponent();

private:
	IComponent* CreateComponent(COMPONENTTYPE type);

private:
	static std::unordered_map<unsigned int, unsigned int>	m_TypeIDs;
	std::vector<std::unique_ptr<IComponent>>				m_Components[IComponent::NUMCOMPONENTTYPE];
};

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
