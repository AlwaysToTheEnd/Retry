#pragma once
#include "GameObject.h"

class DORenderer;
class DOTransform;

class GameLight :public GameObject
{
public:
	GameLight(CGHScene& scene, GameObject* parent, const char* typeName)
		: GameObject(scene, parent, typeName)
		, m_Rrender(nullptr)
		, m_Trans(nullptr)
	{
	}

	virtual			~GameLight() = default;

	void SetDirectionalLight(const physx::PxVec3& color, const physx::PxVec3& dir);
	void SetPointLight(const physx::PxVec3& color, const physx::PxVec3& pos, float fallOffStart, float fallOffEnd);
	void SetSpotLight(const physx::PxVec3& color, const physx::PxVec3& pos, float fallOffStart, float fallOffEndm, float angle);

private:
	virtual void	Update(float delta) override;
	virtual void	Init() override;

private:
	DORenderer* m_Rrender;
	DOTransform* m_Trans;
};