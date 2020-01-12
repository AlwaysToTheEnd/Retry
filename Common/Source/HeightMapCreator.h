#pragma once
#include "GameObject.h"

class HeightMap;
class DOTransform;

class HeightMapCreator :public GameObject
{
public:



private:
	virtual void Update(float delta) override;
	virtual void Init() override;

private:
};