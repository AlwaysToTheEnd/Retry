#pragma once

#include "IComponent.h"
#include <memory>

class ICompnentProvider
{
public:
	virtual std::unique_ptr<IComponent> CreateComponent(COMPONENTTYPE type, PxTransform& trnas)=0;

};