#pragma once
#include "DeviceObject.h"

class IDeviceObjectRegistration
{
public:
	IDeviceObjectRegistration() {};
	virtual void RegisterDeviceObject(CGHScene& scene, DeviceObject* gameObject)=0;
	virtual void UnRegisterDeviceObject(CGHScene& scene, DeviceObject* gameObject) = 0;

private:
};