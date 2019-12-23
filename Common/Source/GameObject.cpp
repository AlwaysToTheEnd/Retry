#include "GameObject.h"
#include "BaseComponent.h"
#include "GraphicComponent.h"
#include "CGHScene.h"
#include "d3dApp.h"


std::unordered_map<unsigned int, unsigned int> GameObject::m_TypeIDs = GameObject::GetComponentTypeIDs();

GameObject::GameObject(CGHScene& scene) 
	:m_Scene(scene)
{

}

std::unordered_map<unsigned int, unsigned int> GameObject::GetComponentTypeIDs()
{
	std::unordered_map<unsigned int, unsigned int> map;

	for (size_t i = 0; i < IComponent::NUMCOMPONENTTYPE; i++)
	{
		COMPONENTTYPE type = static_cast<COMPONENTTYPE>(i);
		size_t hash_code = 0;

		switch (type)
		{
		case COMPONENTTYPE::COM_DYNAMIC:
			hash_code = typeid(ComRigidDynamic).hash_code();
			break;
		case COMPONENTTYPE::COM_STATIC:
			hash_code = typeid(ComRigidStatic).hash_code();
			break;
		case COMPONENTTYPE::COM_UICOLLISTION:
			hash_code = typeid(ComUICollision).hash_code();
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
		case COMPONENTTYPE::COM_ANIMATOR:
			hash_code = typeid(ComAnimator).hash_code();
			break;
		case COMPONENTTYPE::COM_FONT:
			hash_code = typeid(ComFont).hash_code();
			break;
		default:
			assert(false);
			break;
		}

		map.insert({ hash_code, i });
	}

	return map;
}

IComponent* GameObject::CreateComponent(COMPONENTTYPE type)
{
	unsigned int index = static_cast<unsigned int>(type);
	m_Components[index].push_back(m_Scene.CreateComponent(type, *this));

	return m_Components[index].back().get();
}
