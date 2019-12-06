#pragma once
#include "IComponent.h"
class IGraphicDevice;

#include "AnimationObject.h"

class RendererComponent: public IComponent
{
public:
	RendererComponent(PxTransform& transform);
	
	virtual ~RendererComponent();

	virtual void Update() override;

private:
	
};


class AniRendererComponent : public IComponent
{
public:
	AniRendererComponent(PxTransform& transform);

	virtual ~AniRendererComponent();

	virtual void Update() override;
	void AttachMesh();

private:
	Ani::Animation* m_CurrAnimation;
	UINT64			m_Tick;
};
