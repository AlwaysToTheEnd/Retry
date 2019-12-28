#include "AnimationTreeCreator.h"
#include "GraphicComponent.h"
#include "BaseComponent.h"
#include "../UIObjects/UIPanel.h"
#include "d3dApp.h"

using namespace AniTree;

AniNodeVisual::AnimationTreeArrowCreater AniNodeVisual::s_AnitreeArrowCreater;
AniTreeArowVisual::AniTreeArrowArttributeEditer AniTreeArowVisual::s_ArrowArttributeEditer;

void AniNodeVisual::Init()
{
	m_Panel = GetScene().AddGameObject<UIPanel>();
	m_Panel->GetComponent<ComUICollision>()->AddFunc(
		std::bind(&AniNodeVisual::StagingToArrowCreater, this));
	m_Panel->SetSize(100, 100);


#pragma region AddPanel
	{
	}
#pragma endregion

}

void AniNodeVisual::Update()
{
	auto transform = m_Panel->GetComponent<ComTransform>()->GetTransform();

	m_InputPos.x = transform.p.x;
	m_InputPos.y = transform.p.y - 50;

	m_OutputPos.x = transform.p.x;
	m_OutputPos.y = transform.p.y + 50;
}

void AniNodeVisual::Delete()
{
	std::vector<AniTreeArowVisual*>	temp = m_Arrows;

	for (auto& it : temp)
	{
		it->Delete();
	}

	m_Panel->Delete();
	GameObject::Delete();
}

void AniNodeVisual::ArrowVisualDeleted(AniTreeArowVisual* arrow)
{
	for (auto iter = m_Arrows.begin(); iter != m_Arrows.end(); iter++)
	{
		if (*iter == arrow)
		{
			m_Arrows.erase(iter);
			break;
		}
	}
}

void AniNodeVisual::SetTargetAninode(AniTree::AniNode* aniNode)
{
	m_TargetAninode = aniNode;

	auto nodeName = m_TargetAninode->GetNodeName();
	auto nameFont = m_Panel->GetComponent<ComFont>();

	nameFont->m_Text.clear();
	nameFont->m_Text.insert(nameFont->m_Text.end(), nodeName.begin(), nodeName.end());
}

void AniNodeVisual::AddArrow(AniTreeArowVisual* arrow)
{
	m_Arrows.push_back(arrow);
	m_TargetAninode->AddArrow(arrow->GetToNode()->GetNodeName());
}

void AniNodeVisual::StagingToArrowCreater()
{
	s_AnitreeArrowCreater.Excute(this);
}

void AniNodeVisual::AnimationTreeArrowCreater::WorkClear()
{
	if (m_CurrArrow)
	{
		m_CurrArrow->Delete();
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

		m_CurrArrow = m_CurrFrom->GetScene().AddGameObject<AniTreeArowVisual>();
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

void AniTreeArowVisual::Init()
{
	m_Transform = AddComponent<ComTransform>();
	m_Renderer = AddComponent<ComRenderer>();
	m_UICollison = AddComponent<ComUICollision>();


}

void AniTreeArowVisual::Update()
{
	RenderInfo renderInfo(RENDER_UI);
	DirectX::XMFLOAT2 from = m_From->GetOutputPos();
	DirectX::XMFLOAT2 to;
	physx::PxVec2 directionVec;

	if (m_To)
	{
		to = m_To->GetInputPos();
	}
	else
	{
		to = m_MousePos;
	}

	directionVec.x = (to.x - from.x);
	directionVec.y = (to.y - from.y);

	m_Transform->SetTransform(physx::PxTransform(
		physx::PxVec3((directionVec.x) / 2 + from.x, (directionVec.y) / 2 + from.y, m_Transform->GetTransform().p.z),
		physx::PxQuat(physx::PxAtan2(directionVec.x, directionVec.y), physx::PxVec3(0, 0, 1))));

	renderInfo.meshOrTextureName = "plane.png";
	renderInfo.texPoint.size.x = directionVec.magnitude();
	renderInfo.texPoint.size.y = 10;

	m_Renderer->SetRenderInfo(renderInfo);
}

void AniTreeArowVisual::Delete()
{
	if (m_From)
	{
		m_From->ArrowVisualDeleted(this);
	}

	GameObject::Delete();
}

void AniTreeArowVisual::AniTreeArrowArttributeEditer::SetArrowVisual(AniTreeArowVisual* arrow)
{
	WorkStart();

	if (arrow && m_CurrArrow != arrow)
	{
		if (m_CurrArrow)
		{
			WorkClear();
		}

		m_CurrArrow = arrow;
	}
}

void AniTreeArowVisual::AniTreeArrowArttributeEditer::WorkClear()
{
	if (m_AttributePanel)
	{
		m_AttributePanel->Delete();
		m_AttributePanel = nullptr;
	}

	m_CurrArrow = nullptr;
}

void AniTreeArowVisual::AniTreeArrowArttributeEditer::Update()
{

}


















void VisualizedAniTreeCreater::SelectSkinnedData(const std::string& name)
{
	m_CurrSkin = m_Animator->GetSkinnedData(name);
	m_CurrSkin->GetAnimationNames(m_AniNames);
}

void VisualizedAniTreeCreater::Init()
{
	m_Animator = AddComponent<ComAnimator>();
}

void VisualizedAniTreeCreater::Update()
{

}
