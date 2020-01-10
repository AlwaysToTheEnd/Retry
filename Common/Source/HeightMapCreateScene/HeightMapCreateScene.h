#pragma once
#include "CGHScene.h"

class HeightMapCreateScene final :public CGHScene
{
public:
	HeightMapCreateScene(IGraphicDevice* graphicDevice, PhysX4_1* pxDevice)
		:CGHScene(graphicDevice, pxDevice, "HeightMapCreateScene")
	{ }

	virtual void Init() override;
	virtual bool Update(const DirectX::Mouse::ButtonStateTracker& mouse, float delta) override;

private:

};