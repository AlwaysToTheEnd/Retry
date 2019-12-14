#include "AnimationTree.h"

using namespace AniTree;

int AniTree::AniNode::Update(unsigned long long deltaTime)
{
	m_CurrTick += deltaTime;

	for (auto& it : m_Arrows)
	{
		assert((m_RoofAni ? !(it.AniEndIsChange) : true)
			&& "if this animation is roof animation, don't have CHANGE_CONDITION_TYPE_ANI_END");

		if (it.IsTriggersReadyOK(m_CurrTick, m_AniEndTime))
		{
			m_CurrTick = 0;
			return it.targetNodeIndex;
		}
	}

	if (m_RoofAni && m_CurrTick > m_AniEndTime)
	{
		m_CurrTick = 0;
	}
}

bool AniTree::AniNode::AddArrow(unsigned int toNodeIndex, TO_ANI_ARROW_TYPE arrowType, CHANGE_CONDITION_TYPE type, const TriggerData* trigger)
{
	assert((type == CHANGE_CONDITION_TYPE_ANI_END) ? (trigger == nullptr) : (trigger != nullptr));

	int index = -1;

	for (size_t i = 0; i < m_Arrows.size(); i++)
	{
		if (m_Arrows[i].targetNodeIndex == toNodeIndex)
		{
			index = i;
			break;
		}
	}

	if (index == -1)
	{
		index = m_Arrows.size();
		m_Arrows.emplace_back(toNodeIndex);
	}

	if (arrowType != TO_ANI_NODE_TYPE_NONE)
	{
		m_Arrows[index].type = arrowType;
	}

	if (type == CHANGE_CONDITION_TYPE_ANI_END)
	{
		m_Arrows[index].AniEndIsChange = true;
	}
	else
	{
		m_Arrows[index].triggerDatas.push_back(*trigger);
	}

	return true;
}

void AniTree::AnimationTree::Update(unsigned long long deltaTime)
{
	if (m_AniNodes.size())
	{
		int result = m_AniNodes[m_CurrAniNodeIndex].Update(deltaTime);

		if (result != -1)
		{
			m_CurrAniNodeIndex = result;
		}
	}
}

void AniTree::AnimationTree::GetTriggers(std::vector<OutputTrigger>& out)
{
	out.clear();

	for (auto& it : m_AniNodes)
	{
		const std::string& fromNodeName = it.GetAniName();
		auto& arrows = it.GetArrows();

		for (auto& it2 : arrows)
		{
			const std::string& toNodeName = m_AniNodes[it2.targetNodeIndex].GetAniName();

			for (size_t i = 0; i < it2.triggerDatas.size(); i++)
			{
				OutputTrigger temp(fromNodeName, toNodeName, it2.triggerDatas[i]);

				out.push_back(temp);
			}
		}
	}
}

bool AniTree::AnimationTree::AddAniNode(const std::string& aniName, unsigned int aniClipEndTime)
{
	if (GetIndex(aniName) != -1)
	{
		return false;
	}

	m_AniNodes.emplace_back(aniName, aniClipEndTime, m_AniNodes.size());
	return true;
}

bool AniTree::AnimationTree::AddArrow(const std::string& from, const std::string& to,
	TO_ANI_ARROW_TYPE arrowType, CHANGE_CONDITION_TYPE type, const TriggerData* trigger)
{
	int fromNode = GetIndex(from);
	int toNode = GetIndex(to);

	if (fromNode < 0 || toNode < 0)
	{
		return false;
	}

	return m_AniNodes[fromNode].AddArrow(toNode, arrowType, type, trigger);
}

std::string AniTree::AnimationTree::GetCurrAnimationName() const
{
	if (m_CurrAniNodeIndex > -1)
	{
		return m_AniNodes[m_CurrAniNodeIndex].GetAniName();
	}

	return "";
}

unsigned long long AniTree::AnimationTree::GetCurrAnimationTick() const
{
	unsigned long long result = 0;

	if (m_CurrAniNodeIndex > -1)
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

AniTree::TriggerData::TriggerData(TRIGGER_TYPE type, DATA_TYPE dataType, UnionData standard)
	:m_TriggerType(type)
	, m_DataType(dataType)
	, m_Standard(standard)
{
	m_Trigger._i = 0;
}

bool AniTree::TriggerData::IsTriggerOK()
{
	bool result = false;
	TRIGGER_TYPE currFuncTrigger = GetTriggerFuncType();

	switch (m_DataType)
	{
	case AniTree::TYPE_BOOL:
		result = CheckData(currFuncTrigger, m_Trigger._b, m_Standard._b);
		break;
	case AniTree::TYPE_FLOAT:
		result = CheckData(currFuncTrigger, m_Trigger._f, m_Standard._f);
		break;
	case AniTree::TYPE_INT:
		result = CheckData(currFuncTrigger, m_Trigger._i, m_Standard._i);
		break;
	case AniTree::TYPE_UINT:
		result = CheckData(currFuncTrigger, m_Trigger._u, m_Standard._u);
		break;
	}

	if ((m_TriggerType & TRIGGER_TYPE_CHECK_OFF) && result)
	{
		m_Trigger._i = 0;
	}

	return result;
}

TRIGGER_TYPE AniTree::TriggerData::GetTriggerFuncType()
{
	unsigned int currTrigger = m_TriggerType;
	currTrigger &= ~TRIGGER_TYPE_CHECK_OFF;

	return static_cast<TRIGGER_TYPE>(currTrigger);
}

AniTree::NodeArrow::NodeArrow(unsigned int toNodeIndex)
	:targetNodeIndex(toNodeIndex)
	, AniEndIsChange(false)
	, type(TO_ANI_NODE_TYPE_ONE_OK)
{

}

bool AniTree::NodeArrow::IsTriggersReadyOK(unsigned long long currTick, unsigned int clipEnd)
{
	size_t triggerIndex = 0;

	switch (type)
	{
	case AniTree::TO_ANI_NODE_TYPE_ONE_OK:
	{
		if (AniEndIsChange)
		{
			if (currTick >= clipEnd)
			{
				return true;
			}
		}

		for (auto& it : triggerDatas)
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
		if (AniEndIsChange)
		{
			if (currTick < clipEnd)
			{
				return false;
			}
		}

		for (auto& it : triggerDatas)
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
