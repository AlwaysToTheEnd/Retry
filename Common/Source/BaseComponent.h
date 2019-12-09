#pragma once
#include <vector>
#include "IComponent.h"
#include "BaseClass.h"

class ComTransform :public IComponent
{
public:
	ComTransform(GameObject& gameObject, unsigned int index,
		std::vector<CGH::MAT16>* tranMatsPtr);
	virtual ~ComTransform() = default;

	CGH::MAT16& GetMatrix() { return (*m_TransformMats)[m_Index]; }

private:
	virtual void Update() override;

private:
	static std::vector<CGH::MAT16>*	m_TransformMats;

	unsigned int m_Index;
};