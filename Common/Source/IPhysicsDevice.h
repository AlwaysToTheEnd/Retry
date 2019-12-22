#pragma once
#include "IComponentCreater.h"

class CGHScene;

class IPhysicsDevice : public ICompnentCreater
{
public:
	IPhysicsDevice() = default;
	virtual ~IPhysicsDevice() = default;

	virtual bool Init(void* graphicDevicePtr) = 0;
	virtual void Update(const CGHScene& scene) = 0;
	virtual void ExcuteFuncOfClickedObject(float origin_x, float origin_y, float origin_z,
		float ray_x, float ray_y, float ray_z, float distance) = 0;
};
