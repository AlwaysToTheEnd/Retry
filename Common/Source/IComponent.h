#pragma once

#include "foundation/PxTransform.h"

class ComTransform;
using physx::PxTransform;

enum class COMPONENTTYPE
{
	COM_PHYSICS,
	COM_GRAPHIC,
	COM_NONE
};

class IComponent
{
public:
	IComponent(PxTransform& transform) :pTargetTransform(&transform) {};
	virtual ~IComponent() = default;

	virtual void Init() {};
	virtual void Update() {};

private:
	COMPONENTTYPE m_type = COMPONENTTYPE::COM_NONE;
	PxTransform* pTargetTransform = nullptr;
};