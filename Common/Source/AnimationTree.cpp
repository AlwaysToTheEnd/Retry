#include "AnimationTree.h"

using namespace AniTree;
using namespace std;

const AniNode* AniTree::AniNode::Update(float deltaTime)
{
	const AniNode* result = nullptr;

	m_CurrTick += deltaTime * 1000;

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
	return m_TargetAniName;
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

AniTree::NodeArrow* AniTree::AniNode::AddArrow(const AniNode* to)
{
	AniTree::NodeArrow* result = nullptr;

	for (size_t i = 0; i < m_Arrows.size(); i++)
	{
		if (m_Arrows[i].targetNode == to)
		{
			result = &m_Arrows[i];
			break;
		}
	}

	if (result==nullptr)
	{
		m_Arrows.emplace_back(to);
		result = &m_Arrows.back();
	}

	return result;
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

std::ostream& AniTree::operator<<(std::ostream& os, const AniNode& node)
{
	if (node.m_TargetAniName.size())
	{
		os << node.m_TargetAniName << endl;
	}
	else
	{
		os << "#none#" << endl;
	}

	os << node.m_AniEndTime << endl;
	os << node.m_RoofAni << endl;

	os << node.m_Arrows.size() << endl;

	for (auto& it : node.m_Arrows)
	{
		os << it.aniEndIsChange << endl;
		os << it.targetNode->m_IndexFunc() << endl;
		os << it.type << endl;
		os << it.triggers.size() << endl << endl;

		for (auto& trigger : it.triggers)
		{
			os << trigger.m_TriggerType << endl;
			os << static_cast<int>(trigger.m_Standard.type) << endl;

			switch (trigger.m_Standard.type)
			{
			case CGH::DATA_TYPE::TYPE_BOOL:
				os << trigger.m_Standard._b << endl;
				break;
			case CGH::DATA_TYPE::TYPE_FLOAT:
				os << trigger.m_Standard._f << endl;
				break;
			case CGH::DATA_TYPE::TYPE_INT:
				os << trigger.m_Standard._i << endl;
				break;
			case CGH::DATA_TYPE::TYPE_UINT:
				os << trigger.m_Standard._u << endl;
				break;
			}
		}
	}

	return os;
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

void AniTree::AnimationTree::SaveTree(const std::wstring& fileFath)
{
	ofstream save(fileFath.c_str());

	if (save.bad())
	{
		save.close();
		return;
	}

	save.clear();

	if (m_CurrSkinName.size())
	{
		save << m_CurrSkinName << endl;
	}
	else
	{
		save << "#none#" << endl;
	}

	if (m_CurrMeshName.size())
	{
		save << m_CurrMeshName << endl;
	}
	else
	{
		save << "#none#" << endl;
	}

	save << m_AniNodes.size();
	save << endl;

	for (auto& it : m_AniNodes)
	{
		save << (*it);
	}

	save.close();
}

void AniTree::AnimationTree::LoadTree(const std::wstring& fileFath)
{
	ifstream load(fileFath.c_str());

	if (load.bad())
	{
		load.close();
		return;
	}


	load >> m_CurrSkinName;
	load >> m_CurrMeshName;

	if (m_CurrSkinName == "#none#")
	{
		m_CurrSkinName.clear();
	}

	if (m_CurrMeshName == "#none#")
	{
		m_CurrMeshName.clear();
	}

	size_t numAniNodes = 0;
	load >> numAniNodes;

	for (size_t i = 0; i < numAniNodes; i++)
	{
		AddAniNode();
	}

	for (size_t i = 0; i < numAniNodes; i++)
	{
		string name;
		unsigned int time = 0;
		bool isRoofAni = false;
		size_t numArrows = 0;

		load >> name;
		load >> time;
		load >> isRoofAni;
		load >> numArrows;

		if (name != "#none#")
		{
			m_AniNodes[i]->SetAniName(name, time);
		}
		
		m_AniNodes[i]->SetRoofAni(isRoofAni);

		for (size_t i = 0; i < numArrows; i++)
		{
			bool isAniEndChange = false;
			int targetNodeIndex = -1;
			int arrowType = TO_ANI_NODE_TYPE_ONE_OK;
			size_t numTriggers = 0;

			load >> isAniEndChange;
			load >> targetNodeIndex;
			load >> arrowType;
			load >> numTriggers;

			AniNode* toNode = m_AniNodes[targetNodeIndex].get();
			auto currArrow = m_AniNodes[i]->AddArrow(toNode);
			currArrow->aniEndIsChange = isAniEndChange;
			currArrow->type = TO_ANI_ARROW_TYPE(arrowType);
			currArrow->triggers.reserve(numTriggers);

			for (size_t j = 0; j < numTriggers; j++)
			{
				int triggerType = 0;
				int dataType = 0;
				
				load >> triggerType;
				load >> dataType;

				CGH::UnionData standard;
				standard.type = static_cast<CGH::DATA_TYPE>(dataType);

				switch (currArrow->triggers[j].m_Standard.type)
				{
				case CGH::DATA_TYPE::TYPE_BOOL:
					load >> standard._b;
					break;
				case CGH::DATA_TYPE::TYPE_FLOAT:
					load >> standard._f;
					break;
				case CGH::DATA_TYPE::TYPE_INT:
					load >> standard._i;
					break;
				case CGH::DATA_TYPE::TYPE_UINT:
					load >> standard._u;
					break;
				}

				currArrow->triggers.emplace_back(TRIGGER_TYPE(triggerType), standard);
			}
		}
	}

	load.close();
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
	m_AniNodes.back()->SetIndexFunc(std::bind(&AniTree::AnimationTree::GetIndex, this, m_AniNodes.back().get()));
	return m_AniNodes.back().get();
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


