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

void AniNodeVisual::Update(unsigned long long delta)
{

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
	m_Panel->SetSize(100, 60);
	m_Panel->SetBackGroundTexture(InputTN::Get("AniNodeVisualPanel"));

	nameFont->m_Text.clear();
	nameFont->m_Text.insert(nameFont->m_Text.end(), nodeName.begin(), nodeName.end());
}

physx::PxVec2 AniNodeVisual::GetPos() const
{
	auto p = m_Panel->GetComponent<ComTransform>()->GetTransform().p;

	return { p.x, p.y };
}

void AniNodeVisual::SetPos(const physx::PxVec2& pos)
{
	m_Panel->SetPos(pos);
}

const physx::PxVec3& AniNodeVisual::GetSize() const
{
	return m_Panel->GetComponent<ComRenderer>()->GetRenderInfo().point.size;
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

void AniNodeVisual::AnimationTreeArrowCreator::Update(unsigned long long delta)
{
	if (m_CurrFrom)
	{
		if (GETMOUSE(m_CurrFrom->GetConstructor()))
		{
			assert(m_CurrArrow);
			m_isNextClear = false;
			m_CurrArrow->SetCurrMousePos(mouse->GetMousePos());
		}
		else
		{
			WorkClear();
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
	m_Transform->SetPosZ(0.4f);
}

void AniTreeArowVisual::Update(unsigned long long delta)
{
	RenderInfo renderInfo(RENDER_UI);
	physx::PxVec2 from = m_From->GetPos();
	physx::PxVec2 to;
	physx::PxVec2 directionVec;
	physx::PxVec2 directionNormal;
	physx::PxVec2 halfVec;

	if (m_To)
	{
		to = m_To->GetPos();
	}
	else
	{
		to = m_MousePos;
	}

	directionVec.x = (to.x - from.x);
	directionVec.y = (to.y - from.y);
	float angle = physx::PxAtan2(directionVec.y, directionVec.x);

	{
		physx::PxVec2 toToVec = directionVec;
		physx::PxVec3 size = m_From->GetSize();

		float vecYabs = abs(toToVec.y);

		if (vecYabs > size.y)
		{
			toToVec *= (size.y / vecYabs);
		}

		float vecXabs = abs(toToVec.x);
		if (vecXabs > size.x)
		{
			toToVec *= (size.x / vecXabs);
		}

		directionVec -= toToVec;
		from.x += toToVec.x;
		from.y += toToVec.y;
	}

	if (m_To)
	{
		physx::PxVec2 toFromVec = -directionVec;
		physx::PxVec3 size = m_To->GetSize();

		float vecYabs = abs(toFromVec.y);

		if (vecYabs > size.y)
		{
			toFromVec *= (size.y / vecYabs);
		}

		float vecXabs = abs(toFromVec.x);
		if (vecXabs > size.x)
		{
			toFromVec *= (size.x / vecXabs);
		}

		directionVec += toFromVec;
	}

	float halfLength = directionVec.magnitude() / 2.0f;
	halfVec = directionVec / 2;

	physx::PxTransform world(physx::PxVec3(halfVec.x + from.x, halfVec.y + from.y, m_Transform->GetTransform().p.z),
		physx::PxQuat(angle, physx::PxVec3(0, 0, 1)));

	physx::PxVec3 offset = world.q.rotate(physx::PxVec3(0, 10, 0));
	world.p += offset;

	m_Transform->SetTransform(world);

	renderInfo.meshOrTextureName = InputTN::Get("AniTreeArrowVisual");
	renderInfo.texPoint.size.x = halfLength;
	renderInfo.texPoint.size.y = 10;

	if (m_UICollison)
	{
		m_UICollison->SetSize({ halfLength ,10.0f });
	}

	m_Renderer->SetRenderInfo(renderInfo);
}

void AniTreeArowVisual::Delete()
{
	if (m_From)
	{
		if (m_To)
		{
			m_From->GetNode()->DeleteArrow(m_To->GetNodeName());
		}

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
		m_AttributePanel->UIOff();
	}

	m_CurrArrow = nullptr;
}

void AniTreeArowVisual::AniTreeArrowArttributeEditer::Update(unsigned long long delta)
{
	if (!m_CurrArrow)
	{
		WorkEnd();
	}
}

void AniTreeArowVisual::AniTreeArrowArttributeEditer::CreateAttributePanel()
{
	m_CurrArrow->m_From->GetNode()->GetArrows(m_Arrows, m_CurrArrow->m_To->GetNodeName());

	if (m_AttributePanel == nullptr)
	{
		m_AttributePanel = m_CurrArrow->CreateGameObject<UIPanel>(false);

		m_AttributePanel->SetPos(GETAPP->GetMousePos());
		m_AttributePanel->SetBackGroundTexture(InputTN::Get("AniTreeArrowArttributePanel"));
	}

	m_AttributePanel->DeleteAllComs();
	m_AttributePanel->UIOn();

	const int propertyInterval = 2;
	const int objectSetInterval = 8;
	const int offsetX = 5;
	int posY = 5;

	const static std::vector<ENUM_ELEMENT> triggerTypeNames =
	{
		{static_cast<int>(AniTree::TRIGGER_TYPE::TRIGGER_TYPE_GRATER), L"TRIGGER_TYPE_GRATER" },
		{static_cast<int>(AniTree::TRIGGER_TYPE::TRIGGER_TYPE_LESS), L"TRIGGER_TYPE_LESS" },
		{static_cast<int>(AniTree::TRIGGER_TYPE::TRIGGER_TYPE_SAME), L"TRIGGER_TYPE_SAME" },
		{static_cast<int>(AniTree::TRIGGER_TYPE::TRIGGER_TYPE_GRATER | TRIGGER_TYPE_OFF_AFTER_CHECK), L"TRIGGER_TYPE_GRATER_OFF" },
		{static_cast<int>(AniTree::TRIGGER_TYPE::TRIGGER_TYPE_LESS | TRIGGER_TYPE_OFF_AFTER_CHECK), L"TRIGGER_TYPE_LESS_OFF" },
		{static_cast<int>(AniTree::TRIGGER_TYPE::TRIGGER_TYPE_SAME | TRIGGER_TYPE_OFF_AFTER_CHECK), L"TRIGGER_TYPE_SAME_OFF" },
	};

	const static std::vector<ENUM_ELEMENT> dataTypeNames =
	{
		{static_cast<int>(CGH::DATA_TYPE::TYPE_BOOL), L"TYPE_BOOL" },
		{static_cast<int>(CGH::DATA_TYPE::TYPE_FLOAT), L"TYPE_FLOAT" },
		{static_cast<int>(CGH::DATA_TYPE::TYPE_INT), L"TYPE_INT" },
		{static_cast<int>(CGH::DATA_TYPE::TYPE_UINT), L"TYPE_UINT" },
	};

	for (auto& it : m_Arrows)
	{
		auto endIsParam = m_AttributePanel->CreateGameObject<UIParam>(true, UIParam::UIPARAMTYPE::MODIFIER);
		endIsParam->SetTargetParam(L"AniEndIsChange", &it.aniEndIsChange);
		endIsParam->SetTextHeight(m_FontSize);
		m_AttributePanel->AddUICom(offsetX, posY, endIsParam);
		posY += m_FontSize + objectSetInterval;

		for (auto& it2 : it.trigger)
		{
			auto funcParam = m_AttributePanel->CreateGameObject<UIParam>(true, UIParam::UIPARAMTYPE::MODIFIER);
			funcParam->SetTextHeight(m_FontSize);
			funcParam->SetEnumParam(L"Func", &triggerTypeNames, reinterpret_cast<int*>(&it2.m_TriggerType));

			m_AttributePanel->AddUICom(offsetX, posY, funcParam);
			posY += m_FontSize + propertyInterval;

			auto triggerType= m_AttributePanel->CreateGameObject<UIParam>(true, UIParam::UIPARAMTYPE::MODIFIER);
			triggerType->SetTextHeight(m_FontSize);
			triggerType->SetEnumParam(L"DataType", &dataTypeNames,reinterpret_cast<int*>(&it2.m_Standard.type));
			m_AttributePanel->AddUICom(offsetX, posY, triggerType);
			posY += m_FontSize + propertyInterval;

			auto triggerParam = m_AttributePanel->CreateGameObject<UIParam>(true, UIParam::UIPARAMTYPE::MODIFIER);
			triggerParam->SetTextHeight(m_FontSize);

			switch (it2.m_Standard.type)
			{
			case CGH::DATA_TYPE::TYPE_BOOL:
				triggerParam->SetTargetParam(L"BOOL", &it2.m_Standard._b);
				break;
			case CGH::DATA_TYPE::TYPE_FLOAT:
				triggerParam->SetTargetParam(L"FLOAT", &it2.m_Standard._f);
				break;
			case CGH::DATA_TYPE::TYPE_INT:
				triggerParam->SetTargetParam(L"INT", &it2.m_Standard._i);
				break;
			case CGH::DATA_TYPE::TYPE_UINT:
				triggerParam->SetTargetParam(L"UINT", &it2.m_Standard._u);
				break;
			default:
				break;
			}

			triggerParam->GetComponent<ComUICollision>()->AddFunc(std::bind(
				&AniTreeArowVisual::AniTreeArrowArttributeEditer::ChangeType, this, triggerParam, &it2.m_Standard));

			m_AttributePanel->AddUICom(offsetX, posY, triggerParam);
			posY += m_FontSize + objectSetInterval;
		}
	}

	m_AttributePanel->SetSize(230, posY + m_FontSize * 3);

	auto addButton = m_AttributePanel->CreateGameObject<UIButton>(true);
	addButton->SetTexture(
		InputTN::Get("AniTreeArrowArttributePanel_AddButton"),
		{ 10,5 });
	addButton->AddFunc(std::bind(&AniTreeArowVisual::AniTreeArrowArttributeEditer::AddParam, this));

	m_AttributePanel->AddUICom(m_AttributePanel->GetSize().x / 2, posY, addButton);
	posY += m_FontSize + propertyInterval;

	auto deleteButton = m_AttributePanel->CreateGameObject<UIButton>(true);
	deleteButton->SetTexture(
		InputTN::Get("AniTreeArrowArttributePanel_DeleteButton"),
		{ 10,5 });
	deleteButton->AddFunc([this]()
		{
			m_CurrArrow->Delete();
			this->WorkClear();
		});

	m_AttributePanel->AddUICom(m_AttributePanel->GetSize().x / 2, posY, deleteButton);
}

void AniTreeArowVisual::AniTreeArrowArttributeEditer::AddParam()
{
	if (m_Arrows.front().trigger.size() < 10)
	{
		CGH::UnionData test;
		test.type = CGH::DATA_TYPE::TYPE_INT;
		test._i = 0;
		m_Arrows.front().trigger.emplace_back(TRIGGER_TYPE::TRIGGER_TYPE_SAME, test);

		CreateAttributePanel();
	}
}

void AniTreeArowVisual::AniTreeArrowArttributeEditer::ChangeType(UIParam* target, CGH::UnionData* data)
{
	switch (data->type)
	{
	case CGH::DATA_TYPE::TYPE_BOOL:
		target->SetTargetParam(L"BOOL", &data->_b);
		break;
	case CGH::DATA_TYPE::TYPE_FLOAT:
		target->SetTargetParam(L"FLOAT", &data->_f);
		break;
	case CGH::DATA_TYPE::TYPE_INT:
		target->SetTargetParam(L"INT", &data->_i);
		break;
	case CGH::DATA_TYPE::TYPE_UINT:
		target->SetTargetParam(L"UINT", &data->_u);
		break;
	default:
		break;
	}
}

//////////////
void VisualizedAniTreeCreator::SelectSkinnedData(const std::string& name)
{
	m_CurrSkin = m_Animator->GetSkinnedData(name);
	m_CurrSkin->GetAnimationNames(m_AniNames);

	GetComponent<ComMesh>()->SelectMesh(name);
	m_Renderer->SetActive(true);
	m_Animator->SelectSkin(name);
	m_Animator->SetAnimationTree(true);

	m_Tree = m_Animator->GetAnimationTree();

	m_WorkPanel->DeleteAllComs();

	std::wstring text;
	int i = 0;
	for (auto& it : m_AniNames)
	{
		text.clear();
		text.insert(text.end(), it.begin(), it.end());
		auto button = CreateGameObject<UIButton>(true);
		button->SetText(text);
		button->OnlyFontMode();
		button->SetTextHeight(15);
		button->AddFunc(std::bind(&VisualizedAniTreeCreator::AddNode, this, i));
		m_WorkPanel->AddUICom(50, 15 + i * 15, button);

		i++;
	}
}

void VisualizedAniTreeCreator::Init()
{
	auto trans = AddComponent<ComTransform>();
	AddComponent<ComMesh>();
	m_Renderer = AddComponent<ComRenderer>();
	m_Animator = AddComponent<ComAnimator>();

	m_Renderer->SetRenderInfo(RenderInfo(RENDER_MESH));
	m_Renderer->SetActive(false);

	m_WorkPanel = CreateGameObject<UIPanel>(false);
	m_WorkPanel->SetSize(100, 100);
	m_WorkPanel->SetBackGroundTexture(InputTN::Get("AniTreeCreatorWorkPanel"));
	m_WorkPanel->SetPos({ 50,50 });
}

void VisualizedAniTreeCreator::Update(unsigned long long delta)
{
	
}

void VisualizedAniTreeCreator::AddNode(int aniIndex)
{
	std::string aniName = m_AniNames[aniIndex];
	if (m_Tree->AddAniNode(aniName, m_CurrSkin->GetClipEndTime(aniName), false))
	{
		auto newNode = CreateGameObject<AniNodeVisual>(true);

		newNode->SetTargetAninodeFunc(std::bind(&AnimationTree::GetAniNode, m_Tree, m_AniNames[aniIndex]));
		newNode->SetPos({ 300,100 });
	}
}
