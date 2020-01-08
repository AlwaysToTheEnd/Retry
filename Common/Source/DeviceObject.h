#pragma once
#include "GameObject.h"

class GameObject;
class DeviceObjectUpdater;

class DeviceObject : public GameObject
{
	friend class DeviceObjectUpdater;
public:
	DeviceObject(CGHScene& scene, GameObject* parent, const char* typeName);
	virtual ~DeviceObject() = default;

	virtual void	Delete() override;

	void			SetID(unsigned int id) { m_ID = id; }
	int				GetID() const { return m_ID; }

protected:
	virtual void Update(float delta) = 0;
	virtual void Init() = 0;
	virtual bool IsDeviceObject() const override { return true; }

private:
	int				m_ID;
};