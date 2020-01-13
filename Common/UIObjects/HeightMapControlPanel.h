#pragma once
#include "UIPanel.h"

class HeightMap;
class DOTransform;
class DORenderMesh;
class UIPanel;

class HeightMapControlPanel :public UIPanel
{
public:
	HeightMapControlPanel(CGHScene& scene, GameObject* parent, const char* typeName)
		: UIPanel(scene, parent, typeName)
		, m_CurrHeightMap(nullptr)
		, m_HeightMapScale(1, 1, 1)
	{

	}

	virtual			~HeightMapControlPanel() = default;
	virtual void	Delete() override;

private:
	virtual void	Init() override;

	void			DirtyCall();
	void			DirtyScale();

private:
	HeightMap*					m_CurrHeightMap;
	physx::PxVec3				m_HeightMapScale;
	std::string					m_CurrHeightMapName;
	std::vector<std::string>	m_HeightMapNames;
	std::vector<std::wstring>	m_HeightMapPath;
};