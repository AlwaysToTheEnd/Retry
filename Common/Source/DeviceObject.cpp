#include "DeviceObject.h"
#include "CGHScene.h"

DeviceObject::DeviceObject(CGHScene& scene, GameObject* const parent, unsigned int hashCode)
	: GameObject(scene, parent, hashCode, false)
	, m_ID(-1)
{
	scene.RegisterDeviceObject(hashCode, this);
}

void DeviceObject::Delete()
{
	for (auto& it : m_Components)
	{
		it->Delete();
	}

	if (m_Parent)
	{
		m_Parent->ExceptComponent(this);
	}

	m_Scene.UnRegisterDeviceObject(m_HashCode, this);
}
