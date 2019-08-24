#pragma once

#include "IComponent.h"
#include <vector>
#include <memory>

class D3DApp;

class GameObject
{
public:
	GameObject() = default;
	virtual ~GameObject() = default;

	virtual void Init()=0;
	virtual void Update()=0;
	
protected:
	IComponent* AddComponent(COMPONENTTYPE type);
	
protected:
	PxTransform m_transform;
	std::vector<std::unique_ptr<IComponent>> m_components;
};