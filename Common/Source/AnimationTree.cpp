#include "AnimationTree.h"

using namespace AniTree;

std::string AniTree::AniNode::Update(unsigned long long deltaTime)
{
	std::string result = m_NodeName;

	m_CurrTick += deltaTime;

	if (m_RoofAni && m_CurrTick > m_AniEndTime)
	{
		m_CurrTick = 0;
	}

	for (auto& it : m_Arrows)
	{
		assert((m_RoofAni ? !(it.aniEndIsChange) : true)
			&& "if this animation is roof animation, don't have CHANGE_CONDITION_TYPE_ANI_END");

		if (CheckArrowTrigger(it, it.triggers, m_CurrTick, m_AniEndTime))
		{
			m_CurrTick = 0;
			result = it.targetNode;
			break;
		}
	}

	return result;
}

const std::string& AniTree::AniNode::GetAniName() const
{
	if (m_TargetAniName.length())
	{
		return m_TargetAniName;
	}
	else
	{
		return m_NodeName;
	}
}

void AniTree::AniNode::GetArrows(std::vector<OutputArrow>& out, const std::string& to)
{
	out.clear();
	if (to.size() == 0)
	{
		for (auto& it : m_Arrows)
		{
			out.emplace_back(m_NodeName, it.targetNode, it.triggers, it.aniEndIsChange, it.type);
		}
	}
	else
	{
		for (auto& it : m_Arrows)
		{
			if (it.targetNode == to)
			{
				out.emplace_back(m_NodeName, it.targetNode, it.triggers, it.aniEndIsChange, it.type);
				break;
			}
		}
	}
}

void AniTree::AniNode::AddArrow(const std::string& to)
{
	int arrowIndex = -1;

	for (size_t i = 0; i < m_Arrows.size(); i++)
	{
		if (m_Arrows[i].targetNode == to)
		{
			arrowIndex = i;
			break;
		}
	}

	if (arrowIndex == -1)
	{
		arrowIndex = m_Arrows.size();
		m_Arrows.emplace_back(to);
	}
}

bool AniTree::AniNode::AddTrigger(const std::string& to, TO_ANI_ARROW_TYPE arrowType, CHANGE_CONDITION_TYPE type, const TriggerData* trigger)
{
	assert((type == CHANGE_CONDITION_TYPE_ANI_END) ? (trigger == nullptr) : (trigger != nullptr));

	int arrowIndex = -1;

	for (size_t i = 0; i < m_Arrows.size(); i++)
	{
		if (m_Arrows[i].targetNode == to)
		{
			arrowIndex = i;
			break;
		}
	}

	if (arrowIndex == -1)
	{
		arrowIndex = m_Arrows.size();
		m_Arrows.emplace_back(to);

		assert(arrowType != TO_ANI_NODE_TYPE_USING_PREV_TYPE);
	}

	if (arrowType != TO_ANI_NODE_TYPE_USING_PREV_TYPE)
	{
		m_Arrows[arrowIndex].type = arrowType;
	}

	if (type == CHANGE_CONDITION_TYPE_ANI_END)
	{
		m_Arrows[arrowIndex].aniEndIsChange = true;
	}
	else
	{
		m_Arrows[arrowIndex].triggers.push_back(*trigger);
	}


	return true;
}

void AniTree::AniNode::TriggerReset()
{
	for (auto& it : m_Arrows)
	{
		for (auto& it2 : it.triggers)
		{
			if (!(it2.m_TriggerType & TRIGGER_TYPE_OFF_AFTER_CHECK))
			{
				it2.m_Trigger._i = 0;
			}
		}
	}
}

bool AniTree::AniNode::CheckArrowTrigger(NodeArrow& arrow, std::vector<TriggerData>& triggers, unsigned long long currTick, unsigned long long aniEndTick)
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

bool AniTree::AnimationTree::Update(unsigned long long deltaTime)
{
	if (m_AniNodes.size())
	{
		std::string result = m_AniNodes[m_CurrAniNodeIndex].Update(deltaTime);

		if (result != m_AniNodes[m_CurrAniNodeIndex].GetNodeName())
		{
			m_CurrAniNodeIndex = GetIndex(result);
		}

		for (auto& it : m_AniNodes)
		{
			it.TriggerReset();
		}

		return true;
	}

	return false;
}

void AniTree::AnimationTree::GetArrows(std::vector<OutputArrow>& out)
{
	out.clear();

	for (size_t nodeIndex = 0; nodeIndex < m_AniNodes.size(); nodeIndex++)
	{
		m_AniNodes[nodeIndex].GetArrows(out);
	}
}

bool AniTree::AnimationTree::AddAniNode(const std::string& aniName, unsigned int aniClipEndTime, bool roof)
{
	if (GetIndex(aniName) != -1)
	{
		return false;
	}

	m_AniNodes.emplace_back(aniName, aniClipEndTime, roof, m_AniNodes.size());
	return true;
}

AniNode* AniTree::AnimationTree::GetAniNode(const std::string aniName)
{
	for (auto& it : m_AniNodes)
	{
		if (it.GetAniName() == aniName)
		{
			return &it;
		}
	}

	return nullptr;
}

bool AniTree::AnimationTree::AddTrigger(const std::string& from, const std::string& to,
	TO_ANI_ARROW_TYPE arrowType, CHANGE_CONDITION_TYPE type, const TriggerData* trigger)
{
	bool result = false;

	int fromNode = GetIndex(from);

	if (fromNode > -1 && GetIndex(to) > -1)
	{
		result = m_AniNodes[fromNode].AddTrigger(to, arrowType, type, trigger);
	}

	return result;
}

bool AniTree::AnimationTree::AddTrigger(unsigned int from, unsigned int to, TO_ANI_ARROW_TYPE arrowType, CHANGE_CONDITION_TYPE type, const TriggerData* trigger)
{
	bool result = false;

	if (from > -1 && to > -1 && from != to)
	{
		result = m_AniNodes[from].AddTrigger(m_AniNodes[to].GetNodeName(), arrowType, type, trigger);
	}

	return result;
}

std::string AniTree::AnimationTree::GetCurrAnimationName() const
{
	if (m_AniNodes.size())
	{
		return m_AniNodes[m_CurrAniNodeIndex].GetAniName();
	}

	return "";
}

unsigned long long AniTree::AnimationTree::GetCurrAnimationTick() const
{
	unsigned long long result = 0;

	if (m_AniNodes.size())
	{
		result = m_AniNodes[m_CurrAniNodeIndex].GetCurrTick();
	}

	return result;
}

int AniTree::AnimationTree::GetIndex(const std::string& aniName) const
{
	int index = 0;
	for (auto& it : m_AniNodes)
	{
		if (it.GetAniName() == aniName)
		{
			return index;
		}

		index++;
	}

	index = -1;
	return index;
}

bool AniTree::AnimationTree::CheckArrowTrigger(NodeArrow& arrow, std::vector<TriggerData>& triggers,
	unsigned long long currTick, unsigned long long aniEndTick)
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

AniTree::TriggerData::TriggerData(TRIGGER_TYPE type, CGH::UnionData standard)
	:m_TriggerType(type)
	, m_Standard(standard)
{
	m_Trigger._i = 0;
}

bool AniTree::TriggerData::IsTriggerOK()
{
	bool result = false;
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

	if ((m_TriggerType & TRIGGER_TYPE_OFF_AFTER_CHECK))
	{
		if (result)
		{
			m_Trigger._i = 0;
		}
	}

	return result;
}

TRIGGER_TYPE AniTree::TriggerData::GetTriggerFuncType()
{
	unsigned int currTrigger = m_TriggerType;
	currTrigger &= ~TRIGGER_TYPE_OFF_AFTER_CHECK;

	return static_cast<TRIGGER_TYPE>(currTrigger);
}

AniTree::NodeArrow::NodeArrow(const std::string& toNodeName)
	:targetNode(toNodeName)
	, aniEndIsChange(false)
	, type(TO_ANI_NODE_TYPE_ONE_OK)
{
	triggers.reserve(15);
}