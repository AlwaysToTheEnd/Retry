#include "GraphicComponent.h"
#include "GameObject.h"

ComRenderer::ComRenderer(GameObject& gameObject)
	:IComponent(gameObject,COMPONENTTYPE::COM_RENDERER)
{

}

void ComRenderer::Update()
{

}