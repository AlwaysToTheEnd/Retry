#pragma once

class IPhysicsDevice
{
	friend class D3DApp;

public:
	IPhysicsDevice() = default;
	virtual ~IPhysicsDevice() = default;

protected:
	virtual bool Init(void* graphicDevicePtr) = 0;
	virtual void Update() = 0;
};
