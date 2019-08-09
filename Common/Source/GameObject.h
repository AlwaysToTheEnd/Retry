#pragma once

#include "IComponent.h"
#include <vector>
#include <memory>

class GameObject
{
public:
	void AddComponent(COMPONENTTYPE type);

private:
	PxTransform m_transform;
	std::vector<std::shared_ptr<IComponent>> m_components;
};