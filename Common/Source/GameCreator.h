#pragma once
#include "GameObject.h"


class GameCreator :public GameObject
{
public:
	GameCreator(CGHScene& scene, GameObject* parent, const char* typeName)
		: GameObject(scene, parent, typeName)
		, m_CurrHeightMap(nullptr)
		, m_MainControlPanel(nullptr)
		, m_Scale(1,1,1)
	{

	}

	virtual			~GameCreator() = default;
	virtual void	Delete() override;

private:
	virtual void	Update(float delta) override;
	virtual void	Init() override;

	void			DirtyCall();
	void			DirtyScale();

private:
	HeightMap*					m_CurrHeightMap;
	UIPanel*					m_MainControlPanel;
	physx::PxVec3				m_Scale;
	std::string					m_CurrHeightMapName;
	std::vector<std::string>	m_HeightMapNames;
	std::vector<std::wstring>	m_HeightMapPath;
};