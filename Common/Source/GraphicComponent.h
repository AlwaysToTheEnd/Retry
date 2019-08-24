#pragma once
#include "IComponent.h"
class IGraphicDevice;

class GraphicComponent: public IComponent
{
public:
	GraphicComponent(PxTransform& transform, IGraphicDevice* device);
	
	virtual ~GraphicComponent();

	virtual void Update() override;

private:
	IGraphicDevice* m_GDevice;
};

