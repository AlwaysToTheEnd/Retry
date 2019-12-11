#pragma once
#include <vector>
#include <DirectXMath.h>
#include "IComponent.h"
#include "BaseClass.h"

class ComTransform :public IComponent
{
public:
	ComTransform(GameObject& gameObject, int ID,
		std::vector<CGH::MAT16>* tranMatsPtr)
		: IComponent(COMPONENTTYPE::COM_TRANSFORM, gameObject, ID)
		, m_Pos(0,0,0)
		, m_Quter(0,0,0,0)
	{
		if (m_TransformMats == nullptr)
		{
			m_TransformMats = tranMatsPtr;
		}
	}

	const CGH::MAT16& GetMatrix() { return (*m_TransformMats)[GetID()]; }

private:
	static std::vector<CGH::MAT16>* m_TransformMats;

	DirectX::XMFLOAT3 m_Pos;
	DirectX::XMFLOAT4 m_Quter;
};

class ComPhysics :public IComponent
{
public:
	ComPhysics(GameObject& gameObject, int ID);

	virtual void Update() override;

private:
};