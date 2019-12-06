#include "GraphicComponent.h"
#include <comip.h>
#include "IGraphicDevice.h"

RendererComponent::RendererComponent(PxTransform& transform) 
	: IComponent(transform, COMPONENTTYPE::COM_GRAPHIC)
{
	
}

RendererComponent::~RendererComponent()
{

}

void RendererComponent::Update()
{

}
