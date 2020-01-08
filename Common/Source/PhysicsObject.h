#pragma once
#include "DeviceObject.h"

class PhyscisObject : public DeviceObject
{
public:
	PhyscisObject(CGHScene& scene, GameObject* parent, const char* typeName)
		:DeviceObject(scene, parent, typeName)
	{

	}
	virtual ~PhyscisObject() = default;

	virtual bool	IsObjectType(GAMEOBJECT_TYPE type) const override { return type & (DEVICE_OBJECT | PHYSICS_OBJECT); }

protected:
	virtual void	Update(float delta) = 0;
};