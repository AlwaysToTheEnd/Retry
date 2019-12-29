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
		std::bind(&AniNodeVisual::AnimationTreeArrowCreater::Excute, &s_AnitreeArrowCreater, this));

#pragma region AddPanel
	{
	}
#pragma endregion

}

void AniNodeVisual::Update()
{
	auto transform = m_Panel->GetComponent<ComTransform>()->GetTransform();

	float panelXHalfSize = m_Panel->GetSize().x / 2.0f;
	m_InputPos.y = transform.p.y;
	m_InputPos.x = transform.p.x - panelXHalfSize;

	m_OutputPos.y = transform.p.y;
	m_OutputPos.x = transform.p.x + panelXHalfSize;
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

void AniNodeVisual::SetTargetAninodeFunc(std::function<AniTree::AniNode*(void)> func)
{
	m_GetTargetAninodeFunc = func;

	auto nodeName = func()->GetNodeName();
	auto nameFont = m_Panel->GetComponent<ComFont>();
	m_Panel->SetSize(70, 45);
	m_Panel->SetBackGroundColor({ 0,1,0,1 });

	nameFont->m_Text.clear();
	nameFont->m_Text.insert(nameFont->m_Text.end(), nodeName.begin(), nodeName.end());
}

void AniNodeVisual::AddArrow(AniTreeArowVisual* arrow)
{
	m_Arrows.push_back(arrow);
	m_GetTargetAninodeFunc()->AddArrow(arrow->GetToNode()->GetNodeName());
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
	if (m_CurrFrom)
	{
		if (GETMOUSE(m_CurrFrom->m_Panel))
		{
			assert(m_CurrArrow);

			m_CurrArrow->SetCurrMousePos(mouse->GetMousePos());
		}
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
	physx::PxVec2 halfVec;

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
	halfVec = directionVec / 2;

	m_Transform->SetTransform(physx::PxTransform(
		physx::PxVec3(halfVec.x + from.x, halfVec.y + from.y, m_Transform->GetTransform().p.z),
		physx::PxQuat(atan2f(halfVec.y, halfVec.x), physx::PxVec3(0,0,1))));

	renderInfo.meshOrTextureName = "plane.png";
	renderInfo.texPoint.size.x = directionVec.magnitude()/2.0f;
	renderInfo.texPoint.size.y = 5;

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

//////////////
void VisualizedAniTreeCreater::SelectSkinnedData(const std::string& name)
{
	m_CurrSkin = m_Animator->GetSkinnedData(name);
	m_CurrSkin->GetAnimationNames(m_AniNames);

	m_Tree = nullptr;
	m_Tree = std::make_unique<AniTree::AnimationTree>();

	m_WorkPanel->DeleteAllComs();

	std::wstring text;
	int i = 0;
	for (auto& it : m_AniNames)
	{
		text.clear();
		text.insert(text.end(), it.begin(), it.end());
		auto button = GetScene().AddGameObject<UIButton>();
		button->SetTexture("closeButton.png", { 15,15 });
		button->SetText(text);
		button->SetTextHeight(10);
		button->AddFunc(std::bind(&VisualizedAniTreeCreater::AddNode, this, i));
		button->GetComponent<ComFont>()->SetBenchmark(RenderFont::FONTBENCHMARK::CENTER);
		m_WorkPanel->AddUICom(15, 15+i*30, button);

		i++;
	}
	
}

void VisualizedAniTreeCreater::Init()
{
	m_Animator = AddComponent<ComAnimator>();
	m_WorkPanel = GetScene().AddGameObject<UIPanel>();
	m_WorkPanel->SetSize(100, 100);
	m_WorkPanel->SetBackGroundColor({ 0,0,1,1 });
}

void VisualizedAniTreeCreater::Update()
{

}

void VisualizedAniTreeCreater::AddNode(int aniIndex)
{
	std::string aniName = m_AniNames[aniIndex];
	if (m_Tree->AddAniNode(aniName, m_CurrSkin->GetClipEndTime(aniName), false))
	{
		auto newNode = GetScene().AddGameObject<AniNodeVisual>();

		newNode->SetTargetAninodeFunc(std::bind(&AnimationTree::GetAniNode, m_Tree.get(), m_AniNames[aniIndex]));
	}
}
