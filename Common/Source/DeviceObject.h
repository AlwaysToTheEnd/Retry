#pragma once
#include "GameObject.h"

class GameObject;
class DeviceObjectUpdater;
class IGraphicDevice;
class ISoundDevice;
class PhysX4_1;

class DeviceObject : public GameObject
{
	friend class DeviceObjectUpdater;
public:
	DeviceObject(CGHScene& scene, GameObject* parent, const char* typeName);
	virtual ~DeviceObject() = default;

	virtual void	Delete() override;

	void			SetDeviceOBID(unsigned int id) { m_DeviceOBID = id; }
	int				GetDeviceOBID() const { return m_DeviceOBID; }
	virtual bool	IsObjectType(GAMEOBJECT_TYPE type) const override { return type & DEVICE_OBJECT; }
	virtual void	Init(IGraphicDevice* graphicDevice, ISoundDevice* soundDevice, PhysX4_1* physxDevice) = 0;

protected:
	virtual void	Update(float delta) = 0;
	void			Init() {}

private:
	int				m_DeviceOBID;
};