#pragma once
#include "IComponent.h"

class ComRenderer :public IComponent
{
public:
	ComRenderer(GameObject& gameObject);
	virtual ~ComRenderer() = default;

private:
	virtual void Update() override;
};