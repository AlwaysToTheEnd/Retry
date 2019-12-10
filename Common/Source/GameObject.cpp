#include "GameObject.h"
#include "d3dApp.h"
#include "BaseComponent.h"
#include "GraphicComponent.h"

std::unordered_map<unsigned int, unsigned int> GameObject::m_TypeIDs;

GameObject::GameObject()
{
	if (m_TypeIDs.size() == 0)
	{
		for (size_t i = 0; i < NUMCOMPONENTTYPE; i++)
		{
			COMPONENTTYPE type = static_cast<COMPONENTTYPE>(i);
			size_t hash_code = 0;

			switch (type)
			{
			case COMPONENTTYPE::COM_PHYSICS:
				hash_code = typeid(ComPhysics).hash_code();
				break;
			case COMPONENTTYPE::COM_TRANSFORM:
				hash_code = typeid(ComTransform).hash_code();
				break;
			case COMPONENTTYPE::COM_RENDERER:
				hash_code = typeid(ComRenderer).hash_code();
				break;
			case COMPONENTTYPE::COM_MESH:
				hash_code = typeid(ComMesh).hash_code();
				break;
			case COMPONENTTYPE::COM_ANIMATER:
				hash_code = typeid(ComAnimater).hash_code();
				break;
			default:
				assert(false);
				break;
			}

			m_TypeIDs.insert({ hash_code, i });
		}
	}
}

IComponent* GameObject::AddComponent(COMPONENTTYPE type)
{
	unsigned int index = static_cast<unsigned int>(type);
	m_Components[index].push_back(GETAPP->CreateComponent(type, *this));

	return m_Components[index].back().get();
}
