#pragma once
#include "IComponent.h"

class PhysXFunctionalObject
{
public:
	PhysXFunctionalObject(const GameObject* object)
		:m_GameObject(object)
	{

	}

	bool IsValideObject()
	{
		return Dogtag == 123456789;
	}

private:
	int Dogtag = 123456789;

public:
	std::vector<std::function<void()>>	m_VoidFuncs;
	const GameObject*					m_GameObject;
};


struct UICollisions
{
	UICollisions(const GameObject* object, 
		const physx::PxTransform& t, 
		const DirectX::XMFLOAT2& s,
		const std::vector<std::function<void()>>& f)
		:gameObject(object)
		,transform(t)
		,size(s)
		,voidFuncs(f)
	{
	
	}

	const physx::PxTransform& transform;
	const DirectX::XMFLOAT2& size;
	const std::vector<std::function<void()>>& voidFuncs;
	const void* gameObject;
};