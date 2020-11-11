#include "DeviceObject.h"
#include "CGHScene.h"

DeviceObject::DeviceObject(CGHScene& scene, GameObject* parent, const char* typeName)
	: GameObject(scene, parent, typeName)
	, m_DeviceOBID(-1)
{

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

	m_Scene.UnRegisterDeviceObject(this);
}
