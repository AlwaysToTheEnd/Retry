#pragma once
#include "IComponentCreater.h"

class IPhysicsDevice : public ICompnentCreater
{
public:
	IPhysicsDevice() = default;
	virtual ~IPhysicsDevice() = default;

	virtual bool Init(void* graphicDevicePtr) = 0;
	virtual void Update() = 0;
	virtual void ExcuteFuncOfClickedObject(float origin_x, float origin_y, float origin_z,
		float ray_x, float ray_y, float ray_z, float distance) = 0;
};
