#include "AnimationTree.h"

using namespace AniTree;

void AniTree::AniNode::Update(unsigned long long deltaTime)
{
	m_CurrTick += deltaTime;

	if (m_RoofAni && m_CurrTick > m_AniEndTime)
	{
		m_CurrTick = 0;
	}
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

bool AniTree::AnimationTree::Update(unsigned long long deltaTime)
{
	if (m_AniNodes.size())
	{
		m_AniNodes[m_CurrAniNodeIndex].Update(deltaTime);
		bool isRoof = m_AniNodes[m_CurrAniNodeIndex].IsRoofAni();
		unsigned long long currTick = m_AniNodes[m_CurrAniNodeIndex].GetCurrTick();
		unsigned long long endTick = m_AniNodes[m_CurrAniNodeIndex].GetEndTick();

		for (size_t arrowIndex = 0; arrowIndex < m_Arrows[m_CurrAniNodeIndex].size(); arrowIndex++)
		{
			NodeArrow& currArrow = m_Arrows[m_CurrAniNodeIndex][arrowIndex];

			assert((isRoof ? !(currArrow.AniEndIsChange) : true)
				&& "if this animation is roof animation, don't have CHANGE_CONDITION_TYPE_ANI_END");
			
			if (CheckArrowTrigger(currArrow, m_Triggers[m_CurrAniNodeIndex][arrowIndex], currTick, endTick))
			{
				m_AniNodes[m_CurrAniNodeIndex].ResetTick();
				m_CurrAniNodeIndex = currArrow.targetNodeIndex;
			}
		}

		TriggerReset();
		return true;
	}

	return false;
}

void AniTree::AnimationTree::GetTriggers(std::vector<OutputTrigger>& out)
{
	out.clear();

	for (size_t nodeIndex = 0; nodeIndex < m_AniNodes.size(); nodeIndex++)
	{
		const std::string& fromNodeName = m_AniNodes[nodeIndex].GetAniName();

		for (size_t arrowIndex = 0; arrowIndex < m_Arrows[nodeIndex].size(); arrowIndex++)
		{
			unsigned int targetNodeIndex = m_Arrows[nodeIndex][arrowIndex].targetNodeIndex;
			const std::string& toNodeName = m_AniNodes[targetNodeIndex].GetAniName();

			for (size_t triggerIndex = 0; triggerIndex < m_Triggers[nodeIndex][arrowIndex].size(); triggerIndex++)
			{
				OutputTrigger temp(fromNodeName, toNodeName, m_Triggers[nodeIndex][arrowIndex][triggerIndex]);

				out.push_back(temp);
			}
		}
	}
}

bool AniTree::AnimationTree::AddAniNode(const std::string& aniName, unsigned int aniClipEndTime, bool roof)
{
	if (GetIndex(aniName) != -1)
	{
		return false;
	}

	m_AniNodes.emplace_back(aniName, aniClipEndTime, roof, m_AniNodes.size());
	m_Arrows.emplace_back();
	m_Triggers.emplace_back();
	return true;
}

bool AniTree::AnimationTree::AddArrow(const std::string& from, const std::string& to,
	TO_ANI_ARROW_TYPE arrowType, CHANGE_CONDITION_TYPE type, const TriggerData* trigger)
{
	int fromNode = GetIndex(from);
	int toNode = GetIndex(to);

	return AddArrow(fromNode, toNode, arrowType, type, trigger);
}

bool AniTree::AnimationTree::AddArrow(unsigned int from, unsigned int to, TO_ANI_ARROW_TYPE arrowType, CHANGE_CONDITION_TYPE type, const TriggerData* trigger)
{
	if (from < 0 || to < 0)
	{
		return false;
	}

	assert((type == CHANGE_CONDITION_TYPE_ANI_END) ? (trigger == nullptr) : (trigger != nullptr));

	int arrowIndex = -1;

	for (size_t i = 0; i < m_Arrows[from].size(); i++)
	{
		if (m_Arrows[from][i].targetNodeIndex == to)
		{
			arrowIndex = i;
			break;
		}
	}

	if (arrowIndex == -1)
	{
		arrowIndex = m_Arrows[from].size();
		m_Arrows[from].emplace_back(to);
		m_Triggers[from].emplace_back();

		assert(arrowType != TO_ANI_NODE_TYPE_USING_PREV_TYPE);
	}

	if (arrowType != TO_ANI_NODE_TYPE_USING_PREV_TYPE)
	{
		m_Arrows[from][arrowIndex].type = arrowType;
	}

	if (type == CHANGE_CONDITION_TYPE_ANI_END)
	{
		m_Arrows[from][arrowIndex].AniEndIsChange = true;
	}
	else
	{
		m_Triggers[from][arrowIndex].push_back(*trigger);
	}


	return true;
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
		if (arrow.AniEndIsChange)
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
		if (arrow.AniEndIsChange)
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

void AniTree::AnimationTree::TriggerReset()
{
	for (auto& it : m_Triggers)
	{
		for (auto& it2 : it)
		{
			for (auto& it3 : it2)
			{
				if (!(it3.m_TriggerType & TRIGGER_TYPE_OFF_AFTER_CHECK))
				{
					it3.m_Trigger._i = 0;
				}
			}
		}
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

AniTree::NodeArrow::NodeArrow(unsigned int toNodeIndex)
	:targetNodeIndex(toNodeIndex)
	, AniEndIsChange(false)
	, type(TO_ANI_NODE_TYPE_ONE_OK)
{

}