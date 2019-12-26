#pragma once
#include "CGHScene.h"

class AniTreeScene final :public CGHScene
{
public:
	AniTreeScene(IGraphicDevice* graphicDevice, IPhysicsDevice* pxDevice)
		:CGHScene(graphicDevice, pxDevice, "AnimationTreeScene")
	{ }

	virtual void Init() override;

private:

};