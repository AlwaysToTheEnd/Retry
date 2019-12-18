#pragma once
#include <memory>
#include "GameObject.h"
#include "ComponentUpdater.h"

class ICompnentCreater
{
public:
	ICompnentCreater();
	virtual std::unique_ptr<IComponent> CreateComponent(COMPONENTTYPE type, GameObject& gameObject)=0;
	virtual void ComponentDeleteManaging(COMPONENTTYPE type, int id) = 0;

protected:
	ComponentUpdater& GetComponentUpdater(COMPONENTTYPE type);

private:
	static ComponentUpdater	m_ComUpdater[IComponent::NUMCOMPONENTTYPE];
};