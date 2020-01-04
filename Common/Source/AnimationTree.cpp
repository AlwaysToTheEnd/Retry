#include "AnimationTree.h"

using namespace AniTree;

const AniNode* AniTree::AniNode::Update(float deltaTime)
{
	const AniNode* result = nullptr;

	m_CurrTick += deltaTime*1000;

	if (m_RoofAni && m_CurrTick > m_AniEndTime)
	{
		m_CurrTick = 0;
	}

	for (auto& it : m_Arrows)
	{
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

void AniTree::AniNode::SetAniName(const std::string& name, unsigned int aniEndTime)
{
	m_TargetAniName = name; 
	m_AniEndTime = aniEndTime;
	m_CurrTick = 0;
}

void AniTree::AniNode::GetArrows(std::vector<OutputArrow>& out, const AniNode* to)
{
	out.clear();
	if (to)
	{
		for (auto& it : m_Arrows)
		{
			out.emplace_back(this, it.targetNode, it.triggers, it.aniEndIsChange, it.type);
		}
	}
	else
	{
		for (auto& it : m_Arrows)
		{
			if (it.targetNode == to)
			{
				out.emplace_back(this, it.targetNode, it.triggers, it.aniEndIsChange, it.type);
				break;
			}
		}
	}
}

void AniTree::AniNode::AddArrow(const AniNode* to)
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

void AniTree::AniNode::DeleteArrow(const AniNode* to)
{
	for (auto iter = m_Arrows.begin(); iter != m_Arrows.end(); iter++)
	{
		if (iter->targetNode == to)
		{
			m_Arrows.erase(iter);
			break;
		}
	}
}

void AniTree::AniNode::DeleteTrigger(const AniNode* to, int index)
{
	for (size_t i = 0; i < m_Arrows.size(); i++)
	{
		if (m_Arrows[i].targetNode == to)
		{
			int currIndex = 0;
			for (auto iter = m_Arrows[i].triggers.begin(); iter != m_Arrows[i].triggers.end(); iter++, currIndex++)
			{
				if (index == currIndex)
				{
					m_Arrows[i].triggers.erase(iter);
					break;
				}
			}

			break;
		}
	}
}

bool AniTree::AniNode::AddTrigger(const AniNode* to, TO_ANI_ARROW_TYPE arrowType, CHANGE_CONDITION_TYPE type, const TriggerData* trigger)
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

bool AniTree::AnimationTree::Update(float deltaTime)
{
	if (m_AniNodes.size())
	{
		const AniNode* result = m_AniNodes[m_CurrAniNodeIndex]->Update(deltaTime);

		if (result)
		{
			m_CurrAniNodeIndex = GetIndex(result);
		}

		for (auto& it : m_AniNodes)
		{
			it->TriggerReset();
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
		m_AniNodes[nodeIndex]->GetArrows(out);
	}
}

AniNode* AniTree::AnimationTree::AddAniNode()
{
	m_AniNodes.push_back(std::make_unique<AniNode>());
	return m_AniNodes.back().get();
}

bool AniTree::AnimationTree::AddTrigger(AniNode* from, const AniNode* to,
	TO_ANI_ARROW_TYPE arrowType, CHANGE_CONDITION_TYPE type, const TriggerData* trigger)
{
	bool result = false;

	if (from && to && from!=to)
	{
		result = from->AddTrigger(to, arrowType, type, trigger);
	}

	return result;
}

std::string AniTree::AnimationTree::GetCurrAnimationName() const
{
	if (m_AniNodes.size())
	{
		return m_AniNodes[m_CurrAniNodeIndex]->GetAniName();
	}

	return "";
}

unsigned long long AniTree::AnimationTree::GetCurrAnimationTick() const
{
	unsigned long long result = 0;

	if (m_AniNodes.size())
	{
		result = m_AniNodes[m_CurrAniNodeIndex]->GetCurrTick();
	}

	return result;
}

void AniTree::AnimationTree::DeleteNode(const AniNode* node)
{
	for (auto iter = m_AniNodes.begin(); iter != m_AniNodes.end(); iter++)
	{
		if (node == (*iter).get())
		{
			m_AniNodes.erase(iter);
			break;
		}
	}
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

int AniTree::AnimationTree::GetIndex(const AniNode* node)
{
	int result = -1;

	int currIndex = 0;
	for (auto& it : m_AniNodes)
	{
		if (it.get() == node)
		{
			result = currIndex;
			break;
		}

		currIndex++;
	}
	
	return result;
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

AniTree::NodeArrow::NodeArrow(const AniNode* to)
	: targetNode(to)
	, aniEndIsChange(false)
	, type(TO_ANI_NODE_TYPE_ONE_OK)
{
	triggers.reserve(10);
}