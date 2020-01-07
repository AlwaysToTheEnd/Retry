#pragma once
#include "GameObject.h"

class GameObject;

class DeviceObject : public GameObject
{
public:
	DeviceObject(CGHScene& scene, GameObject* const parent, unsigned int hashCode);
	virtual ~DeviceObject() = default;

	virtual void	Delete() override;

	void			SetID(unsigned int id) { m_ID = id; }
	int				GetID() { return m_ID; }

protected:
	virtual void Update(float delta) = 0;
	virtual void Init() = 0;

private:
	int				m_ID;
};