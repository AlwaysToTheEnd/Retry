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
}

void AniNodeVisual::Update(float delta)
{

}

void AniNodeVisual::ChangeAniRoof(AniTree::AniNode* node, UIButton* button)
{
	node->SetRoofAni(!node->IsRoofAni());

	button->SetText(L"AniRoof : " + std::wstring(node->IsRoofAni() ? L"true" : L"false"));
}

void AniNodeVisual::ChangedTargetAni()
{
	if (m_CurrAniName.size())
	{
		for (size_t i = 0; i < m_CurrSkinAnimationNames->size(); i++)
		{
			if ((*m_CurrSkinAnimationNames)[i] == m_CurrAniName)
			{
				m_TargetAniNode->SetAniName(m_CurrAniName, (*m_CurrSkinAnimationEndTick)[i]);
				break;
			}
		}
	}
}

void AniNodeVisual::Delete()
{
	std::vector<AniTreeArowVisual*>	temp = m_Arrows;

	for (auto& it : temp)
	{
		it->Delete();
	}

	m_Panel->Delete();

	m_DeleteAninodeFunc();
	GameObject::Delete();
}

void AniNodeVisual::DeleteArrow(const AniNodeVisual* to)
{
	for (auto& it : m_Arrows)
	{
		if (it->GetToNodeV() == to)
		{
			it->Delete();
			break;
		}
	}
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

void AniNodeVisual::SetTargetAninode(AniNode* node)
{
	m_TargetAniNode = node;

	m_Panel->SetBackGroundTexture(InputTN::Get("AniNodeVisualPanel"));
	m_Panel->SetSize(100, 60);
	m_Panel->SetPos({ 300,300 });

#pragma region AddPanel
	{
		const int ElementSize = 15;
		m_Panel->DeleteAllComs();

		auto closeButton = m_Panel->CreateGameObject<UIButton>(true);
		closeButton->AddFunc(std::bind(&AniNodeVisual::Delete, this));
		closeButton->SetTexture(InputTN::Get("AniNodeVisualPanel_Delete"), { 10,10 });
		m_Panel->AddUICom(m_Panel->GetSize().x - 10, 10, closeButton);

		auto roofControlButton = m_Panel->CreateGameObject<UIButton>(true);
		roofControlButton->OnlyFontMode();
		roofControlButton->SetTextHeight(ElementSize);
		roofControlButton->SetText(L"AniRoof:" + std::wstring(m_TargetAniNode->IsRoofAni() ? L"true" : L"false"));
		roofControlButton->AddFunc(std::bind(&AniNodeVisual::ChangeAniRoof, this, m_TargetAniNode, roofControlButton));
		m_Panel->AddUICom(m_Panel->GetSize().x / 2, m_Panel->GetSize().y / 2, roofControlButton);

		auto aniNameParam = m_Panel->CreateGameObject<UIParam>(true, UIParam::UIPARAMTYPE::MODIFIER);
		aniNameParam->SetStringParam(L"Animation", m_CurrSkinAnimationNames, &m_CurrAniName);
		aniNameParam->SetDirtyCall(std::bind(&AniNodeVisual::ChangedTargetAni, this));
		aniNameParam->SetTextHeight(ElementSize);

		m_Panel->AddUICom(0, 8, aniNameParam);
	}
#pragma endregion
}

void AniNodeVisual::SetDeleteAniNodeFunc(std::function<void()> func)
{
	m_DeleteAninodeFunc = func;
}

void AniNodeVisual::SetSkinAnimationInfoVectorPtr(const std::vector<std::string>* aniNames, const std::vector<unsigned int>* aniEnds)
{
	m_CurrSkinAnimationNames = aniNames;
	m_CurrSkinAnimationEndTick = aniEnds;
}

physx::PxVec2 AniNodeVisual::GetPos() const
{
	auto p = m_Panel->GetComponent<ComTransform>()->GetTransform().p;

	return { p.x, p.y };
}

const physx::PxVec3& AniNodeVisual::GetSize() const
{
	return m_Panel->GetComponent<ComRenderer>()->GetRenderInfo().point.size;
}

void AniNodeVisual::AddArrow(AniTreeArowVisual* arrow)
{
	m_Arrows.push_back(arrow);
	m_TargetAniNode->AddArrow(arrow->GetToNodeV()->m_TargetAniNode);
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

void AniNodeVisual::AnimationTreeArrowCreator::Update(float delta)
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

void AniTreeArowVisual::Update(float delta)
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
			m_From->GetNode()->DeleteArrow(m_To->GetNode());
			m_To = nullptr;
		}

		m_From->ArrowVisualDeleted(this);
		m_From = nullptr;
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

void AniTreeArowVisual::AniTreeArrowArttributeEditer::Update(float delta)
{
	if (!m_CurrArrow)
	{
		WorkEnd();
	}
}

void AniTreeArowVisual::AniTreeArrowArttributeEditer::CreateAttributePanel()
{
	m_CurrArrow->m_From->GetNode()->GetArrows(m_Arrows, m_CurrArrow->m_To->GetNode());

	if (m_AttributePanel == nullptr)
	{
		m_AttributePanel = m_CurrArrow->CreateGameObject<UIPanel>(false);

		m_AttributePanel->SetPos(physx::PxVec2(GETAPP->GetClientSize().x - 230, 200));
		m_AttributePanel->SetBackGroundTexture(InputTN::Get("AniTreeArrowArttributePanel"));
	}

	m_AttributePanel->DeleteAllComs();
	m_AttributePanel->UIOn();

	const int propertyInterval = 2;
	const int objectSetInterval = 8;
	const int offsetX = 5;
	int posY = 10;

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

	const static std::vector<ENUM_ELEMENT> arrowTypeNames =
	{
		{static_cast<int>(AniTree::TO_ANI_ARROW_TYPE::TO_ANI_NODE_TYPE_ALL_OK), L"ALL_OK" },
		{static_cast<int>(AniTree::TO_ANI_ARROW_TYPE::TO_ANI_NODE_TYPE_ONE_OK), L"ONE_OK" },
	};

	for (auto& it : m_Arrows)
	{
		auto endIsParam = m_AttributePanel->CreateGameObject<UIParam>(true, UIParam::UIPARAMTYPE::MODIFIER);
		endIsParam->SetTargetParam(L"AniEndIsChange", &it.aniEndIsChange);
		endIsParam->SetTextHeight(m_FontSize);
		m_AttributePanel->AddUICom(offsetX, posY, endIsParam);
		posY += m_FontSize + propertyInterval;

		auto arrowTypeParam = m_AttributePanel->CreateGameObject<UIParam>(true, UIParam::UIPARAMTYPE::MODIFIER);
		arrowTypeParam->SetEnumParam(L"ArrowType", &arrowTypeNames, reinterpret_cast<int*>(&it.type));
		arrowTypeParam->SetTextHeight(m_FontSize);
		m_AttributePanel->AddUICom(offsetX, posY, arrowTypeParam);
		posY += m_FontSize + objectSetInterval;

		for (auto& it2 : it.trigger)
		{
			auto funcParam = m_AttributePanel->CreateGameObject<UIParam>(true, UIParam::UIPARAMTYPE::MODIFIER);
			funcParam->SetTextHeight(m_FontSize);
			funcParam->SetEnumParam(L"Func", &triggerTypeNames, reinterpret_cast<int*>(&it2.m_TriggerType));

			m_AttributePanel->AddUICom(offsetX, posY, funcParam);
			posY += m_FontSize + propertyInterval;

			auto triggerType = m_AttributePanel->CreateGameObject<UIParam>(true, UIParam::UIPARAMTYPE::MODIFIER);
			triggerType->SetTextHeight(m_FontSize);
			triggerType->SetEnumParam(L"DataType", &dataTypeNames, reinterpret_cast<int*>(&it2.m_Standard.type));
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
	for (auto& it : m_AniNodeVs)
	{
		it->GetNode()->SetAniName("", 0);
		it->AniNameReset();
	}

	m_AniEndTimes.clear();

	m_CurrSkin = m_Animator->GetSkinnedData(name);
	m_CurrSkin->GetAnimationNames(m_AniNames);

	for (auto& it : m_AniNames)
	{
		m_AniEndTimes.push_back(m_CurrSkin->GetClipEndTime(it));
	}

	GetComponent<ComMesh>()->SelectMesh(name);
	m_Animator->SelectSkin(name);
}

void VisualizedAniTreeCreator::Init()
{
	m_Tree = std::make_unique<AniTree::AnimationTree>();

	AddComponent<ComTransform>();
	AddComponent<ComMesh>();
	m_Renderer = AddComponent<ComRenderer>();
	m_Animator = AddComponent<ComAnimator>();

	m_Animator->SetAnimationTree(m_Tree.get());
	m_Renderer->SetRenderInfo(RenderInfo(RENDER_MESH));

	m_WorkPanel = CreateGameObject<UIPanel>(false);
	m_WorkPanel->SetBackGroundTexture(InputTN::Get("AniTreeCreatorWorkPanel"));
	m_WorkPanel->SetPos({ 50,50 });

	m_Animator->GetSkinNames(m_SkinNames);

	auto button = CreateGameObject<UIButton>(true);
	button->SetText(L"AddNode");
	button->OnlyFontMode();
	button->SetTextHeight(15);
	button->AddFunc(std::bind(&VisualizedAniTreeCreator::AddNode, this));
	m_WorkPanel->AddUICom(50, 15, button);

	int posY = 30;
	for (auto& it : m_SkinNames)
	{
		std::wstring skinName;
		skinName.insert(skinName.end(), it.begin(), it.end());

		auto skinbutton = CreateGameObject<UIButton>(true);
		skinbutton->SetText(skinName);
		skinbutton->OnlyFontMode();
		skinbutton->SetTextHeight(15);
		skinbutton->AddFunc(std::bind(&VisualizedAniTreeCreator::SelectSkinnedData, this, it));
		m_WorkPanel->AddUICom(50, posY, skinbutton);
		posY += 20;
	}

	auto testbutton = CreateGameObject<UIButton>(true);
	testbutton->SetText(L"TestButton");
	testbutton->OnlyFontMode();
	testbutton->SetTextHeight(15);
	testbutton->AddFunc(std::bind(&VisualizedAniTreeCreator::TestCode, this));
	m_WorkPanel->AddUICom(50, posY, testbutton);
	posY += 20;

	m_WorkPanel->SetSize(100, posY);
}

void VisualizedAniTreeCreator::Update(float delta)
{

}

void VisualizedAniTreeCreator::AddNode()
{
	if (auto targetNode = m_Tree->AddAniNode())
	{
		auto newNode = CreateGameObject<AniNodeVisual>(true);
		newNode->SetSkinAnimationInfoVectorPtr(&m_AniNames, &m_AniEndTimes);

		newNode->SetTargetAninode(targetNode);
		newNode->SetDeleteAniNodeFunc(std::bind(&VisualizedAniTreeCreator::DeleteNode, this, newNode));

		m_AniNodeVs.push_back(newNode);
	}
}

void VisualizedAniTreeCreator::DeleteNode(AniNodeVisual* node)
{
	for (auto& it : m_AniNodeVs)
	{
		it->DeleteArrow(node);
	}

	for (auto iter = m_AniNodeVs.begin(); iter != m_AniNodeVs.end(); iter++)
	{
		if ((*iter) == node)
		{
			m_AniNodeVs.erase(iter);
			break;
		}
	}

	m_Tree->DeleteNode(node->GetNode());
}

#define STRICT_TYPED_ITEMIDS

#define NTDDI_VERSION NTDDI_VISTA
#define _WIN32_WINNT _WIN32_WINNT_VISTA

#include <Windows.h>
#include <ShlObj.h>

#pragma comment(lib, "comctl32.lib")
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' "\
                       "version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

void VisualizedAniTreeCreator::TestCode()
{
	IFileOpenDialog* pfsd;
	HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&pfsd));
	if (SUCCEEDED(hr))
	{
		COMDLG_FILTERSPEC const rgSaveTypes[] =
		{
			{ L"Text Documents", L"*.txt" },
			{ L"All Files", L"*.*" },
		};

		hr = pfsd->SetFileTypes(ARRAYSIZE(rgSaveTypes), rgSaveTypes);

		if (SUCCEEDED(hr))
		{
			LPWSTR fileName;
			hr = pfsd->Show(NULL);

			if (SUCCEEDED(hr))
			{
				IShellItem* psi;
				hr = pfsd->GetResult(&psi);
				if (SUCCEEDED(hr))
				{
					psi->GetDisplayName(SIGDN_NORMALDISPLAY, &fileName);
					psi->Release();
					delete[] fileName;
				}
			}
		}

		pfsd->Release();
	}
}
