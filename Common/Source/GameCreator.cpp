#include "GameCreator.h"
#include "GraphicDO.h"
#include "PhysicsDO.h"
#include "d3dUtil.h"
#include "HeightMap.h"
#include "../UIObjects/UIPanel.h"
#include "InputTextureNameList.h"
#include "d3dApp.h"

void GameCreator::Delete()
{
	if (m_CurrHeightMap)
	{
		m_CurrHeightMap->Delete();
		m_CurrHeightMap = nullptr;
	}
}

void GameCreator::Update(float delta)
{

}

void GameCreator::Init()
{
	m_MainControlPanel = CreateComponenet<UIPanel>(false);
	m_MainControlPanel->SetSize(200, 200);
	m_MainControlPanel->SetBackGroundTexture(InputTN::Get("CommonPanelBackground"));

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
	heightMapSelect->SetDirtyCall(std::bind(&GameCreator::DirtyCall, this));

	auto scaleX = CreateComponenet<UIParam>(false, UIParam::UIPARAMTYPE::MODIFIER);
	scaleX->SetTargetParam(L"ScaleX", &m_Scale.x);
	scaleX->SetTextHeight(15);
	scaleX->SetDirtyCall(std::bind(&GameCreator::DirtyScale, this));

	auto scaleY = CreateComponenet<UIParam>(false, UIParam::UIPARAMTYPE::MODIFIER);
	scaleY->SetTargetParam(L"ScaleY", &m_Scale.y);
	scaleY->SetTextHeight(15);
	scaleY->SetDirtyCall(std::bind(&GameCreator::DirtyScale, this));

	auto scaleZ = CreateComponenet<UIParam>(false, UIParam::UIPARAMTYPE::MODIFIER);
	scaleZ->SetTargetParam(L"ScaleZ", &m_Scale.z);
	scaleZ->SetTextHeight(15);
	scaleZ->SetDirtyCall(std::bind(&GameCreator::DirtyScale, this));

	m_MainControlPanel->AddUICom(0, 0, heightMapSelect);
	m_MainControlPanel->AddUICom(0, 0, scaleX);
	m_MainControlPanel->AddUICom(0, 0, scaleY);
	m_MainControlPanel->AddUICom(0, 0, scaleZ);

	m_MainControlPanel->UIComsAlignment({ 10,10 }, { 0, 20 });
	m_MainControlPanel->SetBenchUV({ 1.0f, 0.0f });
	m_MainControlPanel->SetPos(physx::PxVec2(GETAPP->GetClientSize().x, 0));
}

void GameCreator::DirtyCall()
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

					m_CurrHeightMap = CreateComponenet<HeightMap>(false, m_HeightMapPath[i].c_str(), m_Scale);
				}
			}
			else
			{
				m_CurrHeightMap = CreateComponenet<HeightMap>(false, m_HeightMapPath[i].c_str(), m_Scale);
			}

			m_CurrHeightMap->ClearMapPickingWork();
			m_CurrHeightMap->AddMapPickingWrok(std::bind(&GameCreator::CreateObject, this, std::placeholders::_1));
			break;
		}
	}
}

void GameCreator::DirtyScale()
{
	if (m_CurrHeightMap)
	{
		m_CurrHeightMap->SetScale(m_Scale);
	}
}
