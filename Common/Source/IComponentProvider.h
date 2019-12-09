#pragma once

#include "GameObject.h"
#include <memory>

class ICompnentProvider
{
public:
	virtual std::unique_ptr<IComponent> CreateComponent(COMPONENTTYPE type, GameObject& gameObject)=0;
};