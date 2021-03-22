#include "AnimationTreeCreator.h"
#include "GraphicDO.h"
#include "PhysicsDO.h"
#include "SoundDO.h"
#include "../UIObjects/UIPanel.h"
#include "d3dApp.h"
#include "InputTextureNameList.h"
#include "d3dUtil.h"

using namespace AniTree;

void AniNodeVisual::Init()
{
	m_Panel = CreateComponenet<UIPanel>(true);

	m_Panel->SetSize(physx::PxVec2(100, 60));
	m_Panel->SetPos({ 300,300 });

#pragma region AddPanel
	{
		const int ElementSize = 15;
		m_Panel->DeleteAllComs();

		auto closeButton = m_Panel->CreateComponenet<UIButton>(true);
		closeButton->SetTexture(InputTN::Get("AniNodeVisualPanel_Delete"), { 10,10 });
		closeButton->AddFunc(std::bind(&AniNodeVisual::DeleteAniNode, this),
			DirectX::Mouse::ButtonStateTracker::RELEASED);

		m_Panel->AddUICom(closeButton);

		m_RoofControlButton = m_Panel->CreateComponenet<UIButton>(true);
		m_RoofControlButton->OnlyFontMode();
		m_RoofControlButton->SetTextHeight(ElementSize);
		m_Panel->AddUICom(m_RoofControlButton);

		m_AniNameParam = m_Panel->CreateComponenet<UIParam>(true, UIParam::UIPARAMTYPE::MODIFIER);
		m_AniNameParam->SetTextHeight(ElementSize);
		m_Panel->AddUICom(m_AniNameParam);
	}
#pragma endregion

	m_Panel->AddPixelFunc(std::bind(&AniNodeVisual::Excute, this),
		DirectX::Mouse::ButtonStateTracker::RELEASED, DirectX::MOUSEBUTTONINDEX::LEFTBUTTON);
	m_Panel->SetMovedPanel();
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
				m_CurrAniNode->SetAniName(m_CurrAniName, (*m_CurrSkinAnimationEndTick)[i]);
				break;
			}
		}
	}
}

void AniNodeVisual::DeleteAniNode()
{
	if (m_DeleteAninodeFunc)
	{
		m_DeleteAninodeFunc();
	}
}

void AniNodeVisual::Excute()
{
	if (m_ExcuteFunc)
	{
		if (GETAPP->GetMouseHeldTime(DirectX::MOUSEBUTTONINDEX::LEFTBUTTON) < CGH::GO.inOut.nonClickedHeldTime)
		{
			m_ExcuteFunc();
		}
	}
}

void AniNodeVisual::SetRenderValue(AniTree::AniNode* node, std::function<void()> excuteFunc, std::function<void()> deleteFunc)
{
	if (!m_IsActive)
	{
		SetActive(true, true);
	}

	m_CurrAniNode = node;
	m_CurrAniName = node->GetAniName();
	node->SetPos(m_Panel->GetPos());

	m_DeleteAninodeFunc = deleteFunc;
	m_ExcuteFunc = excuteFunc;

	m_RoofControlButton->ClearFunc();
	m_RoofControlButton->SetText(L"AniRoof:" + std::wstring(node->IsRoofAni() ? L"true" : L"false"));
	m_RoofControlButton->AddFunc(std::bind(&AniNodeVisual::ChangeAniRoof, this, node, 
		m_RoofControlButton), DirectX::Mouse::ButtonStateTracker::RELEASED);
}

void AniNodeVisual::SetSkinAnimationInfoVectorPtr(const std::vector<std::string>* aniNames, const std::vector<double>* aniEnds)
{
	m_CurrSkinAnimationNames = aniNames;
	m_CurrSkinAnimationEndTick = aniEnds;

	m_AniNameParam->SetStringParam(L"Animation", m_CurrSkinAnimationNames, &m_CurrAniName);
	m_AniNameParam->SetDirtyCall(std::bind(&AniNodeVisual::ChangedTargetAni, this));
}

physx::PxVec2 AniNodeVisual::GetPos() const
{
	return m_Panel->GetPos();
}

const physx::PxVec2& AniNodeVisual::GetSize() const
{
	return m_Panel->GetSize();
}

void AniNodeVisual::SetPos(const physx::PxVec2& pos)
{
	m_Panel->SetPos(pos);
}

void AniArowVisual::Init()
{
	m_Transform = CreateComponenet<DOTransform>();
	m_Renderer = CreateComponenet<DORenderer>();
	m_Transform->SetPosZ(0.4f);

	m_Renderer->AddPixelFunc(std::bind(&AniArowVisual::Excute, this),
		DirectX::Mouse::ButtonStateTracker::ButtonState::RELEASED, DirectX::MOUSEBUTTONINDEX::LEFTBUTTON);
}

void AniArowVisual::Update(float delta)
{

}

void AniArowVisual::Excute()
{
	if (m_ExcuteFunc && !m_IsIniting)
	{
		m_ExcuteFunc();
	}
}

void AniArowVisual::SetRenderValue(const physx::PxVec2& _from, const physx::PxVec2& _fromSize,
	const physx::PxVec2& _to, const physx::PxVec2& _toSize, std::function<void()> excuteFunc, bool isMousePos)
{
	m_ExcuteFunc = excuteFunc;

	if (!m_IsActive)
	{
		SetActive(true, true);
	}

	m_IsIniting = isMousePos;

	RenderInfo renderInfo(RENDER_2DPLANE);
	physx::PxVec2 from;
	physx::PxVec2 to;
	physx::PxVec2 directionVec;
	physx::PxVec2 directionNormal;
	physx::PxVec2 halfVec;

	from = _from;
	from += _fromSize / 2;

	to = _to;

	if (!isMousePos)
	{
		to += _toSize / 2;
	}

	directionVec.x = (to.x - from.x);
	directionVec.y = (to.y - from.y);
	float angle = physx::PxAtan2(directionVec.y, directionVec.x);

	{
		physx::PxVec2 toToVec = directionVec;
		physx::PxVec2 size = _fromSize / 2;

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

	if (!isMousePos)
	{
		physx::PxVec2 toFromVec = -directionVec;
		physx::PxVec2 size = _toSize / 2;

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
	renderInfo.point.size.x = halfLength;
	renderInfo.point.size.y = 10;

	m_Renderer->SetRenderInfo(renderInfo);
}

VisualizedAniTreeCreator::~VisualizedAniTreeCreator()
{

}

void VisualizedAniTreeCreator::Delete()
{

}

//////////////
void VisualizedAniTreeCreator::SelectSkinnedData(const std::string& name)
{
	if (m_CurrTree)
	{
		m_AniEndTimes.clear();

		m_CurrSkin = m_Animator->GetSkinnedData(name);
		m_CurrSkin->GetAnimationNames(m_AniNames);

		for (auto& it : m_AniNames)
		{
			m_AniEndTimes.push_back(m_CurrSkin->GetClipEndTime(it));
		}

		GetComponent<DORenderMesh>()->SelectMesh(CGH::SKINNED_MESH, name);
		m_Animator->SelectSkin(name);

		m_CurrTree->SetCurrMeshName(name);
		m_CurrTree->SetCurrSkinName(name);
		m_Renderer->SetActive(true);
	}
}

void VisualizedAniTreeCreator::Init()
{
	CreateComponenet<DOTransform>();
	CreateComponenet<DORenderMesh>();
	m_Renderer = CreateComponenet<DORenderer>();
	m_Animator = CreateComponenet<DOAnimator>();
	m_WorkPanel = CreateComponenet<UIPanel>(false);
	m_NullTree = std::make_unique<AnimationTree>();

	m_Renderer->SetRenderInfo(RenderInfo(RENDER_SKIN));
	m_Renderer->GetRenderInfo().cullingBoundSphereRad = 30.0f;

	m_WorkPanel->SetPos({ 50,50 });
	m_WorkPanel->SetName(L"TreeCreatorPanel");

	m_Animator->GetAnimationTreeNames(m_TreeNames);
	m_Animator->GetSkinNames(m_SkinNames);

	int posY = 15;

	auto nullTreeButton = m_WorkPanel->CreateComponenet<UIButton>(true);
	nullTreeButton->SetText(L"AddNullTree");
	nullTreeButton->OnlyFontMode();
	nullTreeButton->SetTextHeight(15);
	nullTreeButton->AddFunc(std::bind(&VisualizedAniTreeCreator::SelectNullTree, this), 
		DirectX::Mouse::ButtonStateTracker::RELEASED);
	m_WorkPanel->AddUICom(nullTreeButton);
	posY += 20;

	SetAnimationTreeListsParamToPanel(10, posY, m_WorkPanel);
	posY += 20;

	auto button = m_WorkPanel->CreateComponenet<UIButton>(true);
	button->SetText(L"AddNode");
	button->OnlyFontMode();
	button->SetTextHeight(15);
	button->AddFunc(std::bind(&VisualizedAniTreeCreator::AddNode, this),
		DirectX::Mouse::ButtonStateTracker::RELEASED);
	m_WorkPanel->AddUICom(button);
	posY += 20;

	for (auto& it : m_SkinNames)
	{
		std::wstring skinName;
		skinName.insert(skinName.end(), it.begin(), it.end());

		auto skinbutton = m_WorkPanel->CreateComponenet<UIButton>(true);
		skinbutton->SetText(skinName);
		skinbutton->OnlyFontMode();
		skinbutton->SetTextHeight(15);
		skinbutton->AddFunc(std::bind(&VisualizedAniTreeCreator::SelectSkinnedData, this, it),
			DirectX::Mouse::ButtonStateTracker::RELEASED);
		m_WorkPanel->AddUICom(skinbutton);
		posY += 20;
	}

	auto testbutton = m_WorkPanel->CreateComponenet<UIButton>(true);
	testbutton->SetText(L"SaveButton");
	testbutton->OnlyFontMode();
	testbutton->SetTextHeight(15);
	testbutton->AddFunc(std::bind(&VisualizedAniTreeCreator::SaveTree, this),
		DirectX::Mouse::ButtonStateTracker::RELEASED);
	m_WorkPanel->AddUICom(testbutton);

	m_Renderer->SetActive(false);
}

void VisualizedAniTreeCreator::Update(float delta)
{
	if (PUSHEDESC)
	{
		CancleExcute();
		m_ArrowAttributePanel->UIOff();
	}

	if (m_NextTimeCancle)
	{
		CancleExcute();
	}

	if (m_CurrTree)
	{
		m_NodePosDatas.clear();
		std::vector<AniTree::AniNode>& nodes = m_CurrTree->GetNodes();
		std::vector<AniTree::AniArrow>& arrows = m_CurrTree->GetArrows();

		for (size_t i = m_AniNodeVs.size(); i < nodes.size(); i++)
		{
			auto newNodeVs = CreateComponenet<AniNodeVisual>(true);
			newNodeVs->SetSkinAnimationInfoVectorPtr(&m_AniNames, &m_AniEndTimes);
			m_AniNodeVs.push_back(newNodeVs);
		}

		for (size_t i = m_AniArowVs.size(); i < arrows.size(); i++)
		{
			auto newArrowVs = CreateComponenet<AniArowVisual>(true);
			m_AniArowVs.push_back(newArrowVs);
		}

		for (size_t i = nodes.size(); i < m_AniNodeVs.size(); i++)
		{
			m_AniNodeVs[i]->SetActive(false, true);
		}

		for (size_t i = arrows.size(); i < m_AniArowVs.size(); i++)
		{
			m_AniArowVs[i]->SetActive(false, true);
		}

		NodePosData nodePosDataTemp;

		for (size_t i = 0; i < nodes.size(); i++)
		{
			m_AniNodeVs[i]->SetRenderValue(&nodes[i],
				std::bind(&VisualizedAniTreeCreator::AniNodeExcute, this, nodes[i]),
				std::bind(&VisualizedAniTreeCreator::DeleteNode, this, &nodes[i]));
			nodePosDataTemp.pos = m_AniNodeVs[i]->GetPos();
			nodePosDataTemp.size = m_AniNodeVs[i]->GetSize();
			m_NodePosDatas.insert({ nodes[i].GetNodeID(), nodePosDataTemp });
		}

		for (size_t i = 0; i < arrows.size(); i++)
		{
			const NodePosData& from = m_NodePosDatas.find(arrows[i].nodeID)->second;

			bool isIniting = m_CurrInitingArrowIndex == i;
			if (!isIniting)
			{
				const NodePosData& to = m_NodePosDatas.find(arrows[i].targetNodeID)->second;
				m_AniArowVs[i]->SetRenderValue(from.pos, from.size, to.pos, to.size,
					std::bind(&VisualizedAniTreeCreator::SetAniArrowAttributePanel, this, &arrows[i]), isIniting);
			}
			else
			{
				m_NextTimeCancle = GETAPP->IsMouseButtonClicked(DirectX::MOUSEBUTTONINDEX::LEFTBUTTON);
				m_CurrMousePos = GETMOUSEPOS;

				m_AniArowVs[i]->SetRenderValue(from.pos, from.size, m_CurrMousePos, {}, nullptr, isIniting);
			}
		}
	}
}

void VisualizedAniTreeCreator::AddNode()
{
	if (m_CurrTree)
	{
		m_CurrTree->AddAniNode();
	}
}

void VisualizedAniTreeCreator::AniNodeExcute(const AniTree::AniNode& node)
{
	std::vector<AniTree::AniArrow>& arrows = m_CurrTree->GetArrows();

	if (m_CurrInitingArrowIndex > -1)
	{
		if (arrows[m_CurrInitingArrowIndex].nodeID == node.GetNodeID() ||
			m_CurrTree->AlreadyHasArrow(arrows[m_CurrInitingArrowIndex].nodeID, node.GetNodeID()))
		{
			CancleExcute();
			return;
		}

		arrows[m_CurrInitingArrowIndex].targetNodeID = node.GetNodeID();
		m_CurrInitingArrowIndex = -1;
	}
	else
	{
		AniTree::AniArrow arrowTemp;
		m_CurrInitingArrowIndex = CGH::SizeTTransINT(arrows.size());

		arrowTemp.nodeID = node.GetNodeID();
		arrows.push_back(arrowTemp);
	}
}

void VisualizedAniTreeCreator::CancleExcute()
{
	if (m_CurrInitingArrowIndex > -1)
	{
		std::vector<AniTree::AniArrow>& arrows = m_CurrTree->GetArrows();
		arrows.pop_back();
		assert(arrows.size() == m_CurrInitingArrowIndex);
		m_CurrInitingArrowIndex = -1;
	}

	m_NextTimeCancle = false;
}

void VisualizedAniTreeCreator::SetAnimationTreeListsParamToPanel(int posX, int posY, UIPanel* workPanel)
{
	if (m_AniTreeParam == nullptr)
	{
		m_AniTreeParam = workPanel->CreateComponenet<UIParam>(true, UIParam::UIPARAMTYPE::MODIFIER);

		m_AniTreeParam->SetTextHeight(15);
		m_AniTreeParam->SetStringParam(L"CurrTree", &m_TreeNames, &m_CurrTreeName);
		m_AniTreeParam->SetDirtyCall(std::bind(&VisualizedAniTreeCreator::ChangedTree, this));

		workPanel->AddUICom(m_AniTreeParam);
	}
}

void VisualizedAniTreeCreator::SetAniArrowAttributePanel(AniTree::AniArrow* arrow)
{
	if (m_ArrowAttributePanel == nullptr)
	{
		m_ArrowAttributePanel = CreateComponenet<UIPanel>(false);

		m_ArrowAttributePanel->SetPos(physx::PxVec2(GETAPP->GetClientSize().x - 230, 200));
		m_ArrowAttributePanel->SetMovedPanel();
	}

	m_ArrowAttributePanel->DeleteAllComs();
	m_ArrowAttributePanel->UIOn();
	m_ArrowAttributePanel->SetSize({});

	const int fontSize = 15;

	const static std::vector<ENUM_ELEMENT> triggerTypeNames =
	{
		{static_cast<int>(AniTree::TRIGGER_TYPE::TRIGGER_TYPE_GRATER), L"TRIGGER_TYPE_GRATER" },
		{static_cast<int>(AniTree::TRIGGER_TYPE::TRIGGER_TYPE_LESS), L"TRIGGER_TYPE_LESS" },
		{static_cast<int>(AniTree::TRIGGER_TYPE::TRIGGER_TYPE_SAME), L"TRIGGER_TYPE_SAME" },
		{static_cast<int>(AniTree::TRIGGER_TYPE::TRIGGER_TYPE_GRATER | TRIGGER_TYPE_DATA_ZEROSET_AFTER_CHECK), L"TRIGGER_TYPE_GRATER_OFF" },
		{static_cast<int>(AniTree::TRIGGER_TYPE::TRIGGER_TYPE_LESS | TRIGGER_TYPE_DATA_ZEROSET_AFTER_CHECK), L"TRIGGER_TYPE_LESS_OFF" },
		{static_cast<int>(AniTree::TRIGGER_TYPE::TRIGGER_TYPE_SAME | TRIGGER_TYPE_DATA_ZEROSET_AFTER_CHECK), L"TRIGGER_TYPE_SAME_OFF" },
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

	auto endIsParam = m_ArrowAttributePanel->CreateComponenet<UIParam>(true, UIParam::UIPARAMTYPE::MODIFIER);
	endIsParam->SetTargetParam(L"AniEndIsChange", &arrow->aniEndIsChange);
	endIsParam->SetTextHeight(fontSize);
	m_ArrowAttributePanel->AddUICom(endIsParam);

	auto arrowTypeParam = m_ArrowAttributePanel->CreateComponenet<UIParam>(true, UIParam::UIPARAMTYPE::MODIFIER);
	arrowTypeParam->SetEnumParam(L"ArrowType", &arrowTypeNames, reinterpret_cast<int*>(&arrow->type));
	arrowTypeParam->SetTextHeight(fontSize);
	m_ArrowAttributePanel->AddUICom(arrowTypeParam);

	for (auto& it : arrow->triggers)
	{
		auto funcParam = m_ArrowAttributePanel->CreateComponenet<UIParam>(true, UIParam::UIPARAMTYPE::MODIFIER);
		funcParam->SetTextHeight(fontSize);
		funcParam->SetEnumParam(L"Func", &triggerTypeNames, reinterpret_cast<int*>(&it.m_TriggerType));

		m_ArrowAttributePanel->AddUICom(funcParam);

		auto triggerType = m_ArrowAttributePanel->CreateComponenet<UIParam>(true, UIParam::UIPARAMTYPE::MODIFIER);
		triggerType->SetTextHeight(fontSize);
		triggerType->SetEnumParam(L"DataType", &dataTypeNames, reinterpret_cast<int*>(&it.m_Standard.type));
		m_ArrowAttributePanel->AddUICom(triggerType);

		auto triggerParam = m_ArrowAttributePanel->CreateComponenet<UIParam>(true, UIParam::UIPARAMTYPE::MODIFIER);
		triggerParam->SetTextHeight(fontSize);

		switch (it.m_Standard.type)
		{
		case CGH::DATA_TYPE::TYPE_BOOL:
			triggerParam->SetTargetParam(L"BOOL", &it.m_Standard._b);
			break;
		case CGH::DATA_TYPE::TYPE_FLOAT:
			triggerParam->SetTargetParam(L"FLOAT", &it.m_Standard._f);
			break;
		case CGH::DATA_TYPE::TYPE_INT:
			triggerParam->SetTargetParam(L"INT", &it.m_Standard._i);
			break;
		case CGH::DATA_TYPE::TYPE_UINT:
			triggerParam->SetTargetParam(L"UINT", &it.m_Standard._u);
			break;
		default:
			break;
		}

		m_ArrowAttributePanel->AddUICom(triggerParam);
	}

	auto addButton = m_ArrowAttributePanel->CreateComponenet<UIButton>(true);
	addButton->SetTexture(
		InputTN::Get("AniTreeArrowArttributePanel_AddButton"),
		{ 10,5 });
	addButton->AddFunc(std::bind(&VisualizedAniTreeCreator::AddParam, this, arrow), 
		DirectX::Mouse::ButtonStateTracker::RELEASED);

	m_ArrowAttributePanel->AddUICom(addButton);

	auto deleteButton = m_ArrowAttributePanel->CreateComponenet<UIButton>(true);
	deleteButton->SetTexture(
		InputTN::Get("AniTreeArrowArttributePanel_DeleteButton"),
		{ 10,5 });
	deleteButton->AddFunc(std::bind(&VisualizedAniTreeCreator::DeleteArrow, this, arrow),
		DirectX::Mouse::ButtonStateTracker::RELEASED);
	m_ArrowAttributePanel->AddUICom(deleteButton);
}

void VisualizedAniTreeCreator::ChangeType(UIParam* target, CGH::UnionData* data)
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

void VisualizedAniTreeCreator::AddParam(AniTree::AniArrow* arrow)
{
	if (arrow->triggers.size() < 10)
	{
		CGH::UnionData data;
		data.type = CGH::DATA_TYPE::TYPE_INT;
		data._i = 0;

		AniTree::TriggerData temp(TRIGGER_TYPE::TRIGGER_TYPE_SAME, data);
		arrow->triggers.push_back(temp);
	}
}

void VisualizedAniTreeCreator::DeleteArrow(AniTree::AniArrow* arrow)
{
	if (m_CurrTree)
	{
		m_CurrTree->DeleteArrow(arrow);
		m_ArrowAttributePanel->UIOff();
	}
}

void VisualizedAniTreeCreator::DeleteNode(AniTree::AniNode* node)
{
	m_CurrTree->DeleteNode(node);

	std::vector<AniTree::AniNode>& nodes = m_CurrTree->GetNodes();

	for (size_t i = 0; i < nodes.size(); i++)
	{
		m_AniNodeVs[i]->SetPos(nodes[i].GetPos());
	}
}

void VisualizedAniTreeCreator::SelectNullTree()
{
	TestSoundPlay();

	m_CurrTree = m_NullTree.get();
	m_Animator->SetAnimationTree(m_CurrTree);
	m_Renderer->SetActive(false);

	m_CurrTreeName.clear();

	std::string skinName = m_CurrTree->GetCurrSkinName();
	std::string meshName = m_CurrTree->GetCurrMeshName();

	if (skinName.size())
	{
		SelectSkinnedData(skinName);
	}

	if (meshName.size())
	{
		GetComponent<DORenderMesh>()->SelectMesh(CGH::SKINNED_MESH, meshName);
	}
}

void VisualizedAniTreeCreator::ChangedTree()
{
	m_Animator->SetAnimationTree(m_CurrTreeName);

	auto tree = m_Animator->GetCurrAnimationTree();

	if (m_CurrTree != tree && tree)
	{
		m_CurrTree = tree;

		std::string skinName = m_CurrTree->GetCurrSkinName();
		std::string meshName = m_CurrTree->GetCurrMeshName();

		if (skinName.size())
		{
			SelectSkinnedData(skinName);
		}

		if (meshName.size())
		{
			GetComponent<DORenderMesh>()->SelectMesh(CGH::SKINNED_MESH, meshName);
		}

		std::vector<AniTree::AniNode>& nodes = m_CurrTree->GetNodes();

		for (size_t i = m_AniNodeVs.size(); i < nodes.size(); i++)
		{
			auto newNodeVs = CreateComponenet<AniNodeVisual>(true);
			newNodeVs->SetSkinAnimationInfoVectorPtr(&m_AniNames, &m_AniEndTimes);
			m_AniNodeVs.push_back(newNodeVs);
		}

		for (size_t i = 0; i < nodes.size(); i++)
		{
			m_AniNodeVs[i]->SetPos(nodes[i].GetPos());
		}
	}
}

#include <Windows.h>
#include <ShlObj.h>
#include <stdio.h>

void VisualizedAniTreeCreator::SaveTree()
{
	IFileSaveDialog* pfsd;
	HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&pfsd));

	if (SUCCEEDED(hr))
	{
		COMDLG_FILTERSPEC const rgSaveTypes[] =
		{
			{ L"Xml Files", L"*.Xml" },
			{ L"All Files", L"*.*" },
		};

		hr = pfsd->SetFileTypes(ARRAYSIZE(rgSaveTypes), rgSaveTypes);
		LPWSTR fileName = nullptr;

		wchar_t buffer[FILENAME_MAX];
		GetCurrentDirectoryW(FILENAME_MAX, buffer);
		std::wstring filePath(buffer);

		if (SUCCEEDED(hr))
		{
			IShellItem* folder;

			while (filePath.size())
			{
				if (filePath.back() == '\\')
				{
					break;
				}
				else
				{
					filePath.pop_back();
				}
			}

			filePath += L"Common\\AniTree";

			ThrowIfFailed(SHCreateItemFromParsingName(filePath.c_str(), nullptr, IID_PPV_ARGS(&folder)));
			ThrowIfFailed(pfsd->SetFolder(folder));
			folder->Release();
		}

		if (SUCCEEDED(hr))
		{
			hr = pfsd->Show(NULL);

			if (SUCCEEDED(hr))
			{
				IShellItem* psi;

				hr = pfsd->GetResult(&psi);
				if (SUCCEEDED(hr))
				{
					ThrowIfFailed(psi->GetDisplayName(SIGDN_NORMALDISPLAY, &fileName));
					std::wstring wFileName(fileName);
					filePath += L"\\" + wFileName+L".Xml";

					if (m_CurrTree)
					{
						std::string treeName(wFileName.begin(), wFileName.end());
						m_CurrTreeName = treeName;

						if (m_CurrTree == m_NullTree.get())
						{
							m_Animator->SaveAnimationTree(filePath, treeName, std::move(m_NullTree));
							m_NullTree = std::make_unique<AnimationTree>();
						}
						else
						{
							m_Animator->SaveAnimationTree(filePath, treeName, m_CurrTree);
						}

						m_Animator->GetAnimationTreeNames(m_TreeNames);
					}

					psi->Release();
				}
			}
		}

		pfsd->Release();
	}
}

void VisualizedAniTreeCreator::TestSoundPlay()
{
	if(!m_Test)
	{
		m_Test = CreateComponenet<DOSound>();
		m_Test->SelectSound(L"./../common/SoundData/test1.wav");
	}

	static float testfloat = 0.5f;

	m_Test->SetVolume(testfloat);
	m_Test->Play(5.0f, false);

	testfloat -= 0.1f;
}
