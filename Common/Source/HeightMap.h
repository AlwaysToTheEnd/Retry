#pragma once
#include "GameObject.h"

class HeightMap :public GameObject
{
public:
	HeightMap(CGHScene& scene, GameObject* parent, const char* typeName)
		: GameObject(scene, parent, typeName)
	{

	}

private:
	virtual void Update(float delta) override;
	virtual void Init() override;
};