#pragma once

#include "foundation/PxTransform.h"

class ComTransform;
using physx::PxTransform;

enum class COMPONENTTYPE
{
	COM_PHYSICS,
	COM_GRAPHIC,
};

class IComponent
{
public:
	IComponent(PxTransform& transform, COMPONENTTYPE type)
		: pTargetTransform(&transform)
		, m_type(type)
	{
	};
	virtual ~IComponent() = default;

	virtual void Update() = 0;
	COMPONENTTYPE GetType() { return m_type; }

private:
	COMPONENTTYPE m_type;
	PxTransform* pTargetTransform = nullptr;
};