#include "GraphicComponent.h"
#include <comip.h>
#include "IGraphicDevice.h"

GraphicComponent::GraphicComponent(PxTransform& transform, IGraphicDevice* device) 
	: IComponent(transform, COMPONENTTYPE::COM_GRAPHIC)
	, m_GDevice(device)
{
	
}

GraphicComponent::~GraphicComponent()
{

}

void GraphicComponent::Update()
{

}
