#pragma once
#include "IComponent.h"

class PhysXFunctionalObject
{
public:
	PhysXFunctionalObject(IComponent* com)
		:m_Component(com)
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
	IComponent*	const					m_Component;
};


struct UICollisions
{
	UICollisions(const physx::PxTransform& t, const DirectX::XMFLOAT2& s,
		const std::vector<std::function<void()>>& f)
		:transform(t)
		,size(s)
		,voidFuncs(f)
	{
	
	}

	const physx::PxTransform& transform;
	const DirectX::XMFLOAT2& size;
	const std::vector<std::function<void()>>& voidFuncs;
};