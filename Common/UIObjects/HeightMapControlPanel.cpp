#include "d3dApp.h"
#include "HeightMapControlPanel.h"
#include "GraphicDO.h"
#include "HeightMap.h"
#include "d3dUtil.h"

void HeightMapControlPanel::Delete()
{
	if (m_CurrHeightMap)
	{
		m_CurrHeightMap->Delete();
		m_CurrHeightMap = nullptr;
	}

	UIPanel::Delete();
}

void HeightMapControlPanel::Init()
{
	UIPanel::Init();
	m_Font->m_Text = L"heightMapCP";

	SetSize(physx::PxVec2(200, 200));
	SetBenchUV({ 1.0f, 0 });
	std::vector<std::wstring> heightMapPath;
	SearchAllFileFromFolder(L"../Common/HeightMap", true, heightMapPath);

	for (auto& it : heightMapPath)
	{
		std::wstring extension;
		std::wstring wfileName = GetFileNameFromPath(it, extension);

		if (extension == L"raw")
		{
			std::string fileName(wfileName.begin(), wfileName.end());
			m_HeightMapPath.push_back(it);
			m_HeightMapNames.push_back(fileName);
		}
	}

	auto heightMapSelect = CreateComponenet<UIParam>(false, UIParam::UIPARAMTYPE::MODIFIER);
	heightMapSelect->SetStringParam(L"CurrHeightMap", &m_HeightMapNames, &m_CurrHeightMapName);
	heightMapSelect->SetTextHeight(15);
	heightMapSelect->SetDirtyCall(std::bind(&HeightMapControlPanel::DirtyCall, this));

	auto scaleX = CreateComponenet<UIParam>(false, UIParam::UIPARAMTYPE::MODIFIER);
	scaleX->SetTargetParam(L"ScaleX", &m_HeightMapScale.x);
	scaleX->SetTextHeight(15);
	scaleX->SetDirtyCall(std::bind(&HeightMapControlPanel::DirtyScale, this));

	auto scaleY = CreateComponenet<UIParam>(false, UIParam::UIPARAMTYPE::MODIFIER);
	scaleY->SetTargetParam(L"ScaleY", &m_HeightMapScale.y);
	scaleY->SetTextHeight(15);
	scaleY->SetDirtyCall(std::bind(&HeightMapControlPanel::DirtyScale, this));

	auto scaleZ = CreateComponenet<UIParam>(false, UIParam::UIPARAMTYPE::MODIFIER);
	scaleZ->SetTargetParam(L"ScaleZ", &m_HeightMapScale.z);
	scaleZ->SetTextHeight(15);
	scaleZ->SetDirtyCall(std::bind(&HeightMapControlPanel::DirtyScale, this));

	AddUICom(heightMapSelect);
	AddUICom(scaleX);
	AddUICom(scaleY);
	AddUICom(scaleZ);

	SetPos(physx::PxVec2(GETAPP->GetClientSize().x, 0));
}

void HeightMapControlPanel::DirtyCall()
{
	for (size_t i = 0; i < m_HeightMapNames.size(); i++)
	{
		if (m_HeightMapNames[i] == m_CurrHeightMapName)
		{
			if (m_CurrHeightMap)
			{
				if (m_CurrHeightMap->GetFileName() != m_CurrHeightMapName)
				{
					m_CurrHeightMap->Delete();
					m_CurrHeightMap = nullptr;

					m_CurrHeightMap = CreateComponenet<HeightMap>(false, m_HeightMapPath[i].c_str(), m_HeightMapScale);
				}
			}
			else
			{
				m_CurrHeightMap = CreateComponenet<HeightMap>(false, m_HeightMapPath[i].c_str(), m_HeightMapScale);
			}

			break;
		}
	}
}

void HeightMapControlPanel::DirtyScale()
{
	if (m_CurrHeightMap)
	{
		m_CurrHeightMap->SetScale(m_HeightMapScale);
	}
}