#pragma once
#include "GameObject.h"

class GameObject;
class DeviceObjectUpdater;
class IGraphicDevice;
class PhysX4_1;

class DeviceObject : public GameObject
{
	friend class DeviceObjectUpdater;
public:
	DeviceObject(CGHScene& scene, GameObject* parent, const char* typeName);
	virtual ~DeviceObject() = default;

	virtual void	Delete() override;

	void			SetID(unsigned int id) { m_ID = id; }
	int				GetID() const { return m_ID; }
	virtual bool	IsObjectType(GAMEOBJECT_TYPE type) const override { return type & DEVICE_OBJECT; }
	virtual void	Init(PhysX4_1* physxDevice, IGraphicDevice* graphicDevice) = 0;

protected:
	virtual void	Update(float delta) = 0;
	void			Init() {}

private:
	int				m_ID;
};