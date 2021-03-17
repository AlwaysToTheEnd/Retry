#pragma once
#include "CGHScene.h"

class HeightMapCreateScene final :public CGHScene
{
public:
	HeightMapCreateScene(IGraphicDevice* graphicDevice, ISoundDevice* audioDevice, PhysX4_1* pxDevice)
		:CGHScene(graphicDevice, audioDevice, pxDevice, "HeightMapCreateScene")
	{ }

	virtual void Init() override;
	virtual void Update(const DirectX::Mouse::ButtonStateTracker& mouse, float delta) override;

private:

};