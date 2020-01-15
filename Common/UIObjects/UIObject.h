#pragma once
#include "GameObject.h"


class UIObject :public GameObject
{
protected:
	enum UITYPE
	{
		UIPANEL,
		UIBUTTON,
		UIPARAM,
	};

public:
	UIObject(CGHScene& scene, GameObject* parent, const char* typeName)
		: GameObject(scene, parent, typeName)
		, m_Size(1,1)
		, m_BenchUV(0.5f,0.5f)
	{
	}
	virtual ~UIObject() = default;

	virtual void	UIOn();
	virtual void	UIOff();

	physx::PxVec2	GetSize() { return m_Size; }

	virtual void	SetPos(const physx::PxVec3& pos) = 0;
	void			SetBenchUV(const physx::PxVec2& benchUV) { m_BenchUV = benchUV; }

protected:
	virtual void	SetActive(bool value, bool components = false) override { GameObject::SetActive(value, components); }

protected:
	physx::PxVec2 m_Size;
	physx::PxVec2 m_BenchUV;
};