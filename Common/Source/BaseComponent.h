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
		, m_Scale(1,1,1)
		, m_Quter(0,0,0,0)
	{
		if (m_TransformMats == nullptr)
		{
			m_TransformMats = tranMatsPtr;
		}
	}

	virtual void Update() override;

public:
	void SetMatrix(CGH::MAT16& mat);

	DirectX::XMFLOAT3 m_Pos;
	DirectX::XMFLOAT3 m_Scale;
	DirectX::XMFLOAT4 m_Quter;

	const CGH::MAT16& GetMatrix() { return (*m_TransformMats)[GetID()]; }
private:
	CGH::MAT16& GetNoneConstMat() { return (*m_TransformMats)[GetID()]; }

private:
	static std::vector<CGH::MAT16>* m_TransformMats;
};

class ComPhysics :public IComponent
{
public:
	ComPhysics(GameObject& gameObject, int ID);

	virtual void Update() override;

private:
};