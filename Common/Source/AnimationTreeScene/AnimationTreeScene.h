#pragma once
#include "CGHScene.h"

class AniTreeScene final :public CGHScene
{
public:
	AniTreeScene(IGraphicDevice* graphicDevice, PhysX4_1* pxDevice)
		:CGHScene(graphicDevice, pxDevice, "AnimationTreeScene")
	{ }

	virtual void Init() override;
	virtual void Update(const DirectX::Mouse::ButtonStateTracker& mouse, float delta) override;

private:

};