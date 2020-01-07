#pragma once
#include "GameObject.h"

class HeightMap :public GameObject
{
public:
	HeightMap(CGHScene& scene)
		:GameObject(scene)
	{

	}

private:
	virtual void Update(float delta) override;
	virtual void Init() override;
};