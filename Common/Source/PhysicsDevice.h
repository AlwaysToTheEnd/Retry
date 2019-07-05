#pragma once

class PhysicsDevice
{
public:
	PhysicsDevice()=default;
	virtual ~PhysicsDevice()=default;

	virtual bool Init(void* graphicDevicePtr) = 0;
	virtual void Update() = 0;
};
