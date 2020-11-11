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