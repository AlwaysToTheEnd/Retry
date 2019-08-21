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
	void AddComponent(D3DApp* app, COMPONENTTYPE type);

protected:
	PxTransform m_transform;
	std::vector<std::shared_ptr<IComponent>> m_components;
};