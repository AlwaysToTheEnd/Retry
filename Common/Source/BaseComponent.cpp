#include "BaseComponent.h"

std::vector<CGH::MAT16>* ComTransform::m_TransformMats = nullptr;

ComTransform::ComTransform(GameObject& gameObject, unsigned int index, std::vector<CGH::MAT16>* tranMatsPtr)
	: IComponent(gameObject, COMPONENTTYPE::COM_TRANSFORM)
	, m_Index(index)
{
	assert(tranMatsPtr);
	if (m_TransformMats == nullptr)
	{
		m_TransformMats = tranMatsPtr;
	}
}

void ComTransform::Update()
{

}
