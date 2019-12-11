#include "BaseComponent.h"

std::vector<CGH::MAT16>* ComTransform::m_TransformMats = nullptr;

ComPhysics::ComPhysics(GameObject& gameObject, int ID)
	:IComponent(COMPONENTTYPE::COM_PHYSICS, gameObject, ID)
{
}

void ComPhysics::Update()
{
}
