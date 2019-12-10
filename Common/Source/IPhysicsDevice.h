#pragma once
#include "IComponentCreater.h"

class IPhysicsDevice : public ICompnentCreater
{
public:
	IPhysicsDevice() = default;
	virtual ~IPhysicsDevice() = default;

	virtual bool Init(void* graphicDevicePtr) = 0;
	virtual void Update() = 0;
};
