#pragma once

class PhysicsDevice
{
public:
	PhysicsDevice()=default;
	virtual ~PhysicsDevice()=default;

	virtual bool Init() = 0;
	virtual void Update() = 0;
};
