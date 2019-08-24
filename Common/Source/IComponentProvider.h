#pragma once

#include "IComponent.h"
#include <memory>

class ICompnentProvider
{
public:
	virtual std::unique_ptr<IComponent> CreateComponent(PxTransform& trnas)=0;

};