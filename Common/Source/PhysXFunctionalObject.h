#pragma once
#include "GameObject.h"

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
		const physx::PxVec2& s,
		const std::vector<std::function<void()>>& f)
		:gameObject(object)
		,transform(t)
		,size(s)
		,voidFuncs(f)
	{
	
	}

	void ExcuteFuncs()
	{
		for (auto& it : voidFuncs)
		{
			it();
		}
	}

	physx::PxTransform		transform;
	const physx::PxVec2&	size;
	const GameObject*		gameObject;

	const std::vector<std::function<void()>>& voidFuncs;
};