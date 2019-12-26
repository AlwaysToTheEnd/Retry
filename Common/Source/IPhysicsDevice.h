#pragma once
#include "IComponentCreater.h"
#include <DirectXMath.h>
#include <foundation/PxTransform.h>
#include <functional>
#include <vector>

class CGHScene;

class IPhysicsDevice : public ICompnentCreater
{
public:
	IPhysicsDevice() = default;
	virtual ~IPhysicsDevice() = default;

	virtual bool Init(void* graphicDevicePtr) = 0;
	virtual void Update(const CGHScene& scene) = 0;
	virtual bool ExcuteFuncOfClickedObject(CGHScene& scene, float origin_x, float origin_y, float origin_z,
		float ray_x, float ray_y, float ray_z, float dist) = 0;
	virtual void CreateScene(const CGHScene& scene) = 0;

protected:

};
