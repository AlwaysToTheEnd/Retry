#pragma once
#include "GameObject.h"

class CGHScene;

class ICompnentCreater
{
public:
	ICompnentCreater() {};
	virtual IComponent* CreateComponent(CGHScene& scene, COMPONENTTYPE type, unsigned int id, GameObject& gameObject)=0;
	virtual void ComponentDeleteManaging(CGHScene& scene, COMPONENTTYPE type, IComponent* deletedCom) = 0;

private:
};