#include "AnimationTree.h"
#include <windows.security.cryptography.h>

using namespace AniTree;
using namespace std;

unsigned int AniTree::AniNode::Update(float deltaTime, std::vector<AniArrow>& arrows)
{
	unsigned int result = 0;

	m_CurrTime += deltaTime;

	if (m_RoofAni && m_CurrTime > m_AniEndTime)
	{
		m_CurrTime = 0;
	}

	for (auto& it : arrows)
	{
		if (it.nodeID == m_NodeID)
		{
			if (CheckArrowTrigger(it, it.triggers, m_CurrTime, m_AniEndTime))
			{
				m_CurrTime = 0;
				result = it.targetNodeID;
				break;
			}
		}
	}

	return result;
}

const std::string& AniTree::AniNode::GetAniName() const
{
	return m_TargetAniName;
}

void AniTree::AniNode::SetAniName(const std::string& name, double aniEndTime)
{
	m_TargetAniName = name;
	m_AniEndTime = aniEndTime;
	m_CurrTime = 0;
}

void AniTree::AniNode::SaveXML(XmlElement* element)
{
	element->SetAttribute(GET_NAME(m_NodeID), m_NodeID);
	element->SetAttribute(GET_NAME(m_TargetAniName), m_TargetAniName.c_str());
	element->SetAttribute(GET_NAME(m_RoofAni), m_RoofAni);
	element->SetAttribute(GET_NAME(m_Pos.x), m_Pos.x);
	element->SetAttribute(GET_NAME(m_Pos.y), m_Pos.y);
}

void AniTree::AniNode::LoadXML(XmlElement* element)
{
	m_NodeID = element->UnsignedAttribute(GET_NAME(m_NodeID));
	m_TargetAniName = element->Attribute(GET_NAME(m_TargetAniName));
	m_RoofAni = element->BoolAttribute(GET_NAME(m_RoofAni));
	m_Pos.x = element->FloatAttribute(GET_NAME(m_Pos.x));
	m_Pos.y = element->FloatAttribute(GET_NAME(m_Pos.y));
}

bool AniTree::AniNode::CheckArrowTrigger(const AniArrow& arrow, std::vector<TriggerData>& triggers, double currTick, double aniEndTick)
{
	switch (arrow.type)
	{
	case AniTree::TO_ANI_NODE_TYPE_ONE_OK:
	{
		if (arrow.aniEndIsChange)
		{
			if (currTick >= aniEndTick)
			{
				return true;
			}
		}

		for (auto& it: triggers)
		{
			if (it.IsTriggerOK() == 1)
			{
				return true;
			}
		}

		return false;
	}
	break;
	case AniTree::TO_ANI_NODE_TYPE_ALL_OK:
	{
		if (arrow.aniEndIsChange)
		{
			if (currTick < aniEndTick)
			{
				return false;
			}
		}

		for (auto& it : triggers)
		{
			if (it.IsTriggerOK() == 0)
			{
				return false;
			}
		}

		return true;
	}
	break;
	default:
	{
		assert(false);
		return false;
	}
	break;
	}
}

bool AniTree::AnimationTree::Update(float deltaTime)
{
	if (m_AniNodes.size())
	{
		unsigned int result = m_AniNodes[m_CurrAniNodeIndex].Update(deltaTime, m_Arrows);

		if (result)
		{
			int index = GetIndex(result);
			assert(index != -1);
			m_CurrAniNodeIndex = index;
		}

		TriggerReset();

		return true;
	}

	return false;
}

void AniTree::AnimationTree::SaveTree(const std::wstring& fileFath)
{
	XmlDocument* document = new XmlDocument;

	Xml::XMLNode* node = document->NewElement("AnimationTree");
	XmlElement* list = nullptr;
	XmlElement* element = nullptr;
	document->InsertFirstChild(node);

	list = document->NewElement("MainData");
	list->SetAttribute(GET_NAME(m_AddedNodeID), m_AddedNodeID);
	list->SetAttribute(GET_NAME(m_CurrSkinName), m_CurrSkinName.c_str());
	list->SetAttribute(GET_NAME(m_CurrMeshName), m_CurrMeshName.c_str());
	node->InsertEndChild(list);

	list = document->NewElement("AniNodes");
	list->SetAttribute("NodeNum", CGH::SizeTTransUINT(m_AniNodes.size()));

	for (auto& it : m_AniNodes)
	{
		element = document->NewElement("AniNode");
		it.SaveXML(element);
		list->InsertEndChild(element);
	}
	node->InsertEndChild(list);
	
	list = document->NewElement("AniArrows");
	list->SetAttribute("ArrowNum", CGH::SizeTTransUINT(m_Arrows.size()));

	for (auto& it : m_Arrows)
	{
		element = document->NewElement("AniArrow");
		it.SaveXML(element, document);
		list->InsertEndChild(element);
	}
	node->InsertEndChild(list);

	Xml::XMLError error = document->SaveFile(std::string(fileFath.begin(), fileFath.end()).c_str());
	assert(error == Xml::XML_SUCCESS);

	delete document;
}

void AniTree::AnimationTree::LoadTree(const std::wstring& fileFath)
{
	XmlDocument* document = new XmlDocument;
	document->LoadFile(std::string(fileFath.begin(), fileFath.end()).c_str());
	assert(!document->Error());

	XmlElement* list = nullptr;
	XmlElement* element = nullptr;

	list = document->FirstChildElement("AnimationTree");
	list = list->FirstChildElement("MainData");
	
	m_AddedNodeID = list->UnsignedAttribute(GET_NAME(m_AddedNodeID));
	m_CurrSkinName = list->Attribute(GET_NAME(m_CurrSkinName));
	m_CurrMeshName = list->Attribute(GET_NAME(m_CurrMeshName));

	list = list->NextSiblingElement("AniNodes");
	
	for (element = list->FirstChildElement("AniNode"); element != nullptr; element = element->NextSiblingElement("AniNode"))
	{
		m_AniNodes.emplace_back();
		m_AniNodes.back().LoadXML(element);
	}

	list = list->NextSiblingElement("AniArrows");

	for (element = list->FirstChildElement("AniArrow"); element != nullptr; element = element->NextSiblingElement("AniArrow"))
	{
		m_Arrows.emplace_back();
		m_Arrows.back().LoadXML(element);
	}

	delete document;
}

std::string AniTree::AnimationTree::GetCurrAnimationName() const
{
	if (m_AniNodes.size())
	{
		return m_AniNodes[m_CurrAniNodeIndex].GetAniName();
	}

	return "";
}

double AniTree::AnimationTree::GetCurrAnimationTime() const
{
	double result = 0;

	if (m_AniNodes.size())
	{
		result = m_AniNodes[m_CurrAniNodeIndex].GetCurrTime();
	}

	return result;
}

bool AniTree::AnimationTree::AlreadyHasArrow(unsigned int fromNodeID, unsigned int toNodeID)
{
	bool result = false;

	for (auto& it : m_Arrows)
	{
		if (fromNodeID == it.nodeID && toNodeID == it.targetNodeID)
		{
			result = true;
			break;
		}
	}

	return result;
}

void AniTree::AnimationTree::AddAniNode()
{
	m_AniNodes.emplace_back(m_AddedNodeID);
	m_AddedNodeID++;
}

void AniTree::AnimationTree::DeleteNode(const AniNode* node)
{
	for (size_t i = 0; i < m_AniNodes.size(); i++)
	{
		if (node == &m_AniNodes[i])
		{
			DeleteArrow(m_AniNodes[i].GetNodeID());

			m_AniNodes[i] = m_AniNodes.back();
			m_AniNodes.pop_back();
			m_CurrAniNodeIndex = 0;
			break;
		}
	}
}

void AniTree::AnimationTree::DeleteArrow(const AniArrow* arrow)
{
	for (size_t i = 0; i < m_Arrows.size(); i++)
	{
		if (arrow == &m_Arrows[i])
		{
			m_Arrows[i] = m_Arrows.back();
			m_Arrows.pop_back();
			break;
		}
	}
}

void AniTree::AnimationTree::DeleteArrow(unsigned int nodeID)
{
	for (size_t i = 0; i < m_Arrows.size(); i++)
	{
		if (nodeID == m_Arrows[i].nodeID || nodeID == m_Arrows[i].targetNodeID)
		{
			m_Arrows[i] = m_Arrows.back();
			m_Arrows.pop_back();
			i--;
		}
	}
}

void AniTree::AnimationTree::SetCurrSkinName(const std::string& name)
{
	if (m_CurrSkinName != name)
	{
		for (auto& it : m_AniNodes)
		{
			it.SetAniName("", 0);
		}

		m_CurrSkinName = name;
	}
}

bool AniTree::AnimationTree::CheckArrowTrigger(AniArrow& arrow, std::vector<TriggerData>& triggers,
	double currTick, double aniEndTick)
{
	switch (arrow.type)
	{
	case AniTree::TO_ANI_NODE_TYPE_ONE_OK:
	{
		if (arrow.aniEndIsChange)
		{
			if (currTick >= aniEndTick)
			{
				return true;
			}
		}

		for (auto& it : triggers)
		{
			if (it.IsTriggerOK())
			{
				return true;
			}
		}

		return false;
	}
	break;
	case AniTree::TO_ANI_NODE_TYPE_ALL_OK:
	{
		if (arrow.aniEndIsChange)
		{
			if (currTick < aniEndTick)
			{
				return false;
			}
		}

		for (auto& it : triggers)
		{
			if (!it.IsTriggerOK())
			{
				return false;
			}
		}

		return true;
	}
	break;
	default:
	{
		assert(false);
		return false;
	}
	break;
	}
}

int AniTree::AnimationTree::GetIndex(unsigned int nodeID)
{
	int result = -1;

	int currIndex = 0;
	for (auto& it : m_AniNodes)
	{
		if (it.GetNodeID() == nodeID)
		{
			result = currIndex;
			break;
		}

		currIndex++;
	}

	return result;
}

AniTree::TriggerData::TriggerData()
{
	ZeroMemory(this, sizeof(AniTree::TriggerData));
}

AniTree::TriggerData::TriggerData(TRIGGER_TYPE type, CGH::UnionData standard)
	:m_TriggerType(type)
	, m_Standard(standard)
{
	ZeroMemory(&m_Trigger, sizeof(CGH::UnionData));
}

int AniTree::TriggerData::IsTriggerOK()
{
	if (m_TriggerType == TRIGGER_TYPE_UNKOWN) return -1;

	int result = 0;
	TRIGGER_TYPE currFuncTrigger = GetTriggerFuncType();

	switch (m_Standard.type)
	{
	case CGH::DATA_TYPE::TYPE_BOOL:
		result = CheckData(currFuncTrigger, m_Trigger._b, m_Standard._b);
		break;
	case CGH::DATA_TYPE::TYPE_FLOAT:
		result = CheckData(currFuncTrigger, m_Trigger._f, m_Standard._f);
		break;
	case CGH::DATA_TYPE::TYPE_INT:
		result = CheckData(currFuncTrigger, m_Trigger._i, m_Standard._i);
		break;
	case CGH::DATA_TYPE::TYPE_UINT:
		result = CheckData(currFuncTrigger, m_Trigger._u, m_Standard._u);
		break;
	}

	if ((m_TriggerType & TRIGGER_TYPE_DATA_ZEROSET_AFTER_CHECK))
	{
		if (result)
		{
			m_Trigger._i = 0;
		}
	}

	return result;
}

void AniTree::TriggerData::SaveXML(XmlElement* element)
{
	element->SetAttribute(GET_NAME(m_TriggerType), m_TriggerType);
	element->SetAttribute(GET_NAME(m_Standard.type), static_cast<int>(m_Standard.type));

	switch (m_Standard.type)
	{
	case CGH::DATA_TYPE::TYPE_BOOL:
		element->SetAttribute(GET_NAME(m_Standard._b), m_Standard._b);
		break;
	case CGH::DATA_TYPE::TYPE_FLOAT:
		element->SetAttribute(GET_NAME(m_Standard._f), m_Standard._f);
		break;
	case CGH::DATA_TYPE::TYPE_INT:
		element->SetAttribute(GET_NAME(m_Standard._i), m_Standard._i);
		break;
	case CGH::DATA_TYPE::TYPE_UINT:
		element->SetAttribute(GET_NAME(m_Standard._u), m_Standard._u);
		break;
	}
}

void AniTree::TriggerData::LoadXML(XmlElement* element)
{
	m_TriggerType = static_cast<AniTree::TRIGGER_TYPE>(element->IntAttribute(GET_NAME(m_TriggerType)));
	m_Standard.type = static_cast<CGH::DATA_TYPE>(element->IntAttribute(GET_NAME(m_Standard.type)));

	switch (m_Standard.type)
	{
	case CGH::DATA_TYPE::TYPE_BOOL:
		m_Standard._b= element->BoolAttribute(GET_NAME(m_Standard._b));
		break;
	case CGH::DATA_TYPE::TYPE_FLOAT:
		m_Standard._f = element->FloatAttribute(GET_NAME(m_Standard._f));
		break;
	case CGH::DATA_TYPE::TYPE_INT:
		m_Standard._i = element->IntAttribute(GET_NAME(m_Standard._i));
		break;
	case CGH::DATA_TYPE::TYPE_UINT:
		m_Standard._u = element->UnsignedAttribute(GET_NAME(m_Standard._u));
		break;
	}
}

TRIGGER_TYPE AniTree::TriggerData::GetTriggerFuncType() const
{
	unsigned int currTrigger = m_TriggerType;
	currTrigger &= ~TRIGGER_TYPE_DATA_ZEROSET_AFTER_CHECK;

	return static_cast<TRIGGER_TYPE>(currTrigger);
}

void AniTree::AnimationTree::TriggerReset()
{
	for (auto& it : m_Arrows)
	{
		for (auto& it2 : it.triggers)
		{
			if (!(it2.m_TriggerType & TRIGGER_TYPE_DATA_ZEROSET_AFTER_CHECK))
			{
				it2.m_Trigger._i = 0;
			}
		}
	}
}

void AniTree::AniArrow::SaveXML(XmlElement* element, Xml::XMLDocument* document)
{
	element->SetAttribute(GET_NAME(type), type);
	element->SetAttribute(GET_NAME(aniEndIsChange), aniEndIsChange);
	element->SetAttribute(GET_NAME(nodeID), nodeID);
	element->SetAttribute(GET_NAME(targetNodeID), targetNodeID);

	for (auto& it : triggers)
	{
		XmlElement* newElement = document->NewElement("Trigger");

		it.SaveXML(newElement);
		element->InsertEndChild(newElement);
	}
}

void AniTree::AniArrow::LoadXML(XmlElement* element)
{
	type = static_cast<TO_ANI_ARROW_TYPE>(element->UnsignedAttribute(GET_NAME(type)));
	aniEndIsChange = element->BoolAttribute(GET_NAME(aniEndIsChange));
	nodeID = element->UnsignedAttribute(GET_NAME(nodeID));
	targetNodeID = element->UnsignedAttribute(GET_NAME(targetNodeID));

	for (XmlElement* trigger = element->FirstChildElement("Trigger"); trigger != nullptr; trigger = trigger->NextSiblingElement("Trigger"))
	{
		triggers.emplace_back();
		triggers.back().LoadXML(element);
	}
}
