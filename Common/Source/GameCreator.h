#pragma once
#include "GameObject.h"

class GameCreator :public GameObject
{
public:
	GameCreator(CGHScene& scene, GameObject* parent, const char* typeName)
		: GameObject(scene, parent, typeName)
	{

	}

	virtual			~GameCreator() = default;
	virtual void	Delete() override;

private:
	virtual void	Update(float delta) override;
	virtual void	Init() override;

	void			SetUI();
private:

};