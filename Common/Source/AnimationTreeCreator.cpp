#include "AnimationTreeCreator.h"
#include "GraphicComponent.h"
#include "BaseComponent.h"
#include "../UIObjects/UIPanel.h"
#include "d3dApp.h"

AniNodeVisual::AnimationTreeArrowCreater AniNodeVisual::s_AnitreeArrowCreater;

void AniNodeVisual::Init()
{
	m_Panel = GetScene().AddGameObject<UIPanel>();
	m_Panel->GetComponent<ComUICollision>()->AddFunc(
		std::bind(&AniNodeVisual::StagingToArrowCreater, this));

}

void AniNodeVisual::Update()
{

}

void AniNodeVisual::AddArrow(AniTreeArowVisual* arrow)
{
	m_Arrows.push_back(arrow);
}

void AniNodeVisual::StagingToArrowCreater()
{
	s_AnitreeArrowCreater.Excute(this);
}

void AniNodeVisual::AnimationTreeArrowCreater::WorkClear()
{
	if (m_CurrArrow)
	{
		m_CurrArrow->GetScene().DeleteGameObject(m_CurrArrow);
		m_CurrArrow = nullptr;
	}

	m_CurrFrom = nullptr;
}

void AniNodeVisual::AnimationTreeArrowCreater::Update()
{
	if (GETMOUSE(m_CurrFrom))
	{
		assert(m_CurrArrow);

		m_CurrArrow->SetCurrMousePos(mouse->GetMousePos());
	}
	else
	{
		WorkClear();
		WorkEnd();
	}
}

void AniNodeVisual::AnimationTreeArrowCreater::Excute(AniNodeVisual* aniNode)
{
	if (!m_CurrFrom)
	{
		WorkStart();
		m_CurrFrom = aniNode;

		m_CurrArrow = m_CurrFrom->GetScene().AddGameObject<AniNodeVisual::AniTreeArowVisual>();
		m_CurrArrow->SetFromNode(m_CurrFrom);
	}
	else
	{
		m_CurrArrow->SetToNode(aniNode);
		m_CurrFrom->AddArrow(m_CurrArrow);
		m_CurrArrow = nullptr;
		m_CurrFrom = nullptr;

		WorkEnd();
	}
}

void AniNodeVisual::AniTreeArowVisual::SetFromNode(AniNodeVisual* from)
{
}

void AniNodeVisual::AniTreeArowVisual::SetCurrMousePos(DirectX::XMFLOAT2 pos)
{
}

void AniNodeVisual::AniTreeArowVisual::SetToNode(AniNodeVisual* to)
{
}

void AniNodeVisual::AniTreeArowVisual::Init()
{
}

void AniNodeVisual::AniTreeArowVisual::Update()
{
}
