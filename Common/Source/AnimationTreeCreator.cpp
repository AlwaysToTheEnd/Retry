#include "AnimationTreeCreator.h"
#include "GraphicComponent.h"
#include "BaseComponent.h"
#include "../UIObjects/UIPanel.h"
#include "d3dApp.h"
#include "InputTextureNameList.h"

using namespace AniTree;

AniNodeVisual::AnimationTreeArrowCreator AniNodeVisual::s_AnitreeArrowCreater;
AniTreeArowVisual::AniTreeArrowArttributeEditer AniTreeArowVisual::s_ArrowArttributeEditer;

void AniNodeVisual::Init()
{
	m_Panel = CreateGameObject<UIPanel>(true);
	m_Panel->GetComponent<ComUICollision>()->AddFunc(
		std::bind(&AniNodeVisual::AnimationTreeArrowCreator::Excute, &s_AnitreeArrowCreater, this));

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

void AniNodeVisual::SetTargetAninodeFunc(std::function<AniTree::AniNode * (void)> func)
{
	m_GetTargetAninodeFunc = func;

	auto nodeName = func()->GetNodeName();
	auto nameFont = m_Panel->GetComponent<ComFont>();
	m_Panel->SetSize(70, 45);
	m_Panel->SetBackGroundTexture(InputTN::Get("AniNodeVisualPanel"));

	nameFont->m_Text.clear();
	nameFont->m_Text.insert(nameFont->m_Text.end(), nodeName.begin(), nodeName.end());
}

void AniNodeVisual::AddArrow(AniTreeArowVisual* arrow)
{
	m_Arrows.push_back(arrow);
	m_GetTargetAninodeFunc()->AddArrow(arrow->GetToNode()->GetNodeName());
}

void AniNodeVisual::AnimationTreeArrowCreator::WorkClear()
{
	if (m_CurrArrow)
	{
		m_CurrArrow->Delete();
		m_CurrArrow = nullptr;
	}

	m_CurrFrom = nullptr;
}

void AniNodeVisual::AnimationTreeArrowCreator::Update()
{
	if (m_CurrFrom)
	{
		if (GETMOUSE(m_CurrFrom->GetConstructor()))
		{
			assert(m_CurrArrow);

			m_CurrArrow->SetCurrMousePos(mouse->GetMousePos());
		}
	}
}

void AniNodeVisual::AnimationTreeArrowCreator::Excute(AniNodeVisual* aniNode)
{
	if (!m_CurrFrom)
	{
		WorkStart();
		m_CurrFrom = aniNode;

		m_CurrArrow = m_CurrFrom->CreateGameObject<AniTreeArowVisual>(true);
		m_CurrArrow->SetFromNode(m_CurrFrom);
	}
	else
	{
		if (m_CurrFrom != aniNode)
		{
			m_CurrArrow->SetToNode(aniNode);
			m_CurrFrom->AddArrow(m_CurrArrow);
		}
		else
		{
			WorkClear();
		}

		m_CurrArrow = nullptr;
		m_CurrFrom = nullptr;

		WorkEnd();
	}
}

void AniTreeArowVisual::Init()
{
	m_Transform = AddComponent<ComTransform>();
	m_Renderer = AddComponent<ComRenderer>();
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
	float halfLength = directionVec.magnitude() / 2.0f;

	m_Transform->SetTransform(physx::PxTransform(
		physx::PxVec3(halfVec.x + from.x, halfVec.y + from.y, m_Transform->GetTransform().p.z),
		physx::PxQuat(atan2f(halfVec.y, halfVec.x), physx::PxVec3(0, 0, 1))));

	renderInfo.meshOrTextureName = InputTN::Get("AniTreeArrowVisual");
	renderInfo.texPoint.size.x = halfLength;
	renderInfo.texPoint.size.y = 5;

	if (m_UICollison)
	{
		m_UICollison->SetSize({ halfLength ,5.0f });
	}

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

void AniTreeArowVisual::SetToNode(AniNodeVisual* to)
{
	if (m_From != to)
	{
		m_To = to;
		m_UICollison = AddComponent<ComUICollision>();
		m_UICollison->AddFunc(
			std::bind(&AniTreeArowVisual::AniTreeArrowArttributeEditer::SetArrowVisual,
				&s_ArrowArttributeEditer, this));
	}
}

void AniTreeArowVisual::AniTreeArrowArttributeEditer::SetArrowVisual(AniTreeArowVisual* arrow)
{
	if (arrow)
	{
		if (arrow->m_To)
		{
			WorkStart();

			WorkClear();

			m_CurrArrow = arrow;
			CreateAttributePanel();
		}
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

void AniTreeArowVisual::AniTreeArrowArttributeEditer::CreateAttributePanel()
{
	m_CurrArrow->m_From->GetNode()->GetArrows(m_Arrows, m_CurrArrow->m_To->GetNodeName());

	m_AttributePanel = m_CurrArrow->CreateGameObject<UIPanel>(false);
	DirectX::XMFLOAT2 pos;
	DirectX::XMStoreFloat2(&pos, GETAPP->GetMousePos());
	m_AttributePanel->SetSize(100, 100);
	m_AttributePanel->SetPos(pos);
	m_AttributePanel->SetBackGroundTexture(InputTN::Get("AniTreeArrowArttributePanel"));

	static bool test = false;
	int posY = 5;
	for (auto& it : m_Arrows)
	{
		auto endIsParam = m_AttributePanel->CreateGameObject<UIParam>(true, UIParam::UIPARAMTYPE::MODIFIER);
		endIsParam->SetTargetParam(L"AniEndIsChange", &it.aniEndIsChange);
		endIsParam->SetTextHeight(m_FontSize);
		m_AttributePanel->AddUICom(10, posY, endIsParam);
		posY += m_FontSize + 2;

		for (auto& it2 : it.trigger)
		{
			auto triggerParam = m_AttributePanel->CreateGameObject<UIParam>(true, UIParam::UIPARAMTYPE::MODIFIER);
			triggerParam->SetTextHeight(m_FontSize);

			switch (it2.m_Standard.type)
			{
			case CGH::DATA_TYPE::TYPE_BOOL:
				triggerParam->SetTargetParam(L"value", &it2.m_Standard._b);
				break;
			case CGH::DATA_TYPE::TYPE_FLOAT:
				triggerParam->SetTargetParam(L"value", &it2.m_Standard._f);
				break;
			case CGH::DATA_TYPE::TYPE_INT:
				triggerParam->SetTargetParam(L"value", &it2.m_Standard._i);
				break;
			case CGH::DATA_TYPE::TYPE_UINT:
				triggerParam->SetTargetParam(L"value", &it2.m_Standard._u);
				break;
			default:
				break;
			}
			m_AttributePanel->AddUICom(10, posY, triggerParam);

			posY += m_FontSize + 2;

			auto funcParam = m_AttributePanel->CreateGameObject<UIParam>(true, UIParam::UIPARAMTYPE::MODIFIER);
			funcParam->SetTextHeight(m_FontSize);
			funcParam->SetTargetParam(L"Type", reinterpret_cast<int*>(&it2.m_TriggerType));

			m_AttributePanel->AddUICom(10, posY, funcParam);

			posY += m_FontSize + 2;
		}

		auto addButton = m_AttributePanel->CreateGameObject<UIButton>(true);
		addButton->SetTexture(
			InputTN::Get("AniTreeArrowArttributePanel_AddButton"), 
			{ 50,5 });
		addButton->AddFunc(std::bind(&AniTreeArowVisual::AniTreeArrowArttributeEditer::AddParam, this));

		m_AttributePanel->AddUICom(m_AttributePanel->GetSize().x/2, posY, addButton);
	}
}

void AniTreeArowVisual::AniTreeArrowArttributeEditer::AddParam()
{
	auto addedParam = m_AttributePanel->CreateGameObject<UIParam>(true, UIParam::UIPARAMTYPE::MODIFIER);
	addedParam->SetTextHeight(m_FontSize);
	CGH::UnionData test;
	test.type = CGH::DATA_TYPE::TYPE_INT;
	test._i = 0;
	m_Arrows.front().trigger.emplace_back(TRIGGER_TYPE::TRIGGER_TYPE_SAME, test);
	addedParam->SetTargetParam(L"test", &m_Arrows.front().trigger.back().m_Standard._i);

	auto funcParam = m_AttributePanel->CreateGameObject<UIParam>(true, UIParam::UIPARAMTYPE::MODIFIER);
	funcParam->SetTextHeight(m_FontSize);
	funcParam->SetTargetParam(L"Type", reinterpret_cast<int*>(&m_Arrows.front().trigger.back().m_TriggerType));

	m_AttributePanel->AddUICom(10, m_AttributePanel->GetNumAddedComs() * (m_FontSize + 2), addedParam);
	m_AttributePanel->AddUICom(10, m_AttributePanel->GetNumAddedComs() * (m_FontSize + 2), funcParam);
}

//////////////
void VisualizedAniTreeCreator::SelectSkinnedData(const std::string& name)
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
		auto button = CreateGameObject<UIButton>(true);
		button->SetTexture(InputTN::Get("AniTreeCreatorWorkPanel_AddButton"), { 15,15 });
		button->SetText(text);
		button->SetTextHeight(10);
		button->AddFunc(std::bind(&VisualizedAniTreeCreator::AddNode, this, i));
		button->GetComponent<ComFont>()->SetBenchmark(RenderFont::FONTBENCHMARK::CENTER);
		m_WorkPanel->AddUICom(15, 15 + i * 30, button);

		i++;
	}
}

void VisualizedAniTreeCreator::Init()
{
	m_Animator = AddComponent<ComAnimator>();
	m_WorkPanel = CreateGameObject<UIPanel>(false);
	m_WorkPanel->SetSize(100, 100);
	m_WorkPanel->SetBackGroundTexture(InputTN::Get("AniTreeCreatorWorkPanel"));
}

void VisualizedAniTreeCreator::Update()
{

}

void VisualizedAniTreeCreator::AddNode(int aniIndex)
{
	std::string aniName = m_AniNames[aniIndex];
	if (m_Tree->AddAniNode(aniName, m_CurrSkin->GetClipEndTime(aniName), false))
	{
		auto newNode = CreateGameObject<AniNodeVisual>(false);

		newNode->SetTargetAninodeFunc(std::bind(&AnimationTree::GetAniNode, m_Tree.get(), m_AniNames[aniIndex]));
	}
}
