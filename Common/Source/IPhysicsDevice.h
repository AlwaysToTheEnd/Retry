#pragma once
#include "IComponentProvider.h"

class IPhysicsDevice : public ICompnentProvider
{
public:
	IPhysicsDevice() = default;
	virtual ~IPhysicsDevice() = default;

	virtual bool Init(void* graphicDevicePtr) = 0;
	virtual void Update() = 0;
};
