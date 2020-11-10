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

std::ostream& AniTree::operator<<(std::ostream& os, const AniNode& node)
{
	if (node.GetNodeID())
	{
		//os << node.m_TargetAniName << endl;
	}
	else
	{
		os << "#none#" << endl;
	}

	//os << node.m_Pos.x << endl;
	//os << node.m_Pos.y << endl;

	//os << node.m_AniEndTime << endl;
	//os << node.m_RoofAni << endl;

	/*os << node.m_Arrows.size() << endl;

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
	}*/

	return os;
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
		save << it;
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
		physx::PxVec2 pos;
		unsigned int time = 0;
		bool isRoofAni = false;
		size_t numArrows = 0;

		load >> name;
		load >> pos.x;
		load >> pos.y;
		load >> time;
		load >> isRoofAni;
		load >> numArrows;

		if (name != "#none#")
		{
			m_AniNodes[i].SetAniName(name, time);
		}

		m_AniNodes[i].SetPos(pos);
		m_AniNodes[i].SetRoofAni(isRoofAni);

		for (size_t j = 0; j < numArrows; j++)
		{
			bool isAniEndChange = false;
			int targetNodeIndex = -1;
			int arrowType = TO_ANI_NODE_TYPE_ONE_OK;
			size_t numTriggers = 0;

			load >> isAniEndChange;
			load >> targetNodeIndex;
			load >> arrowType;
			load >> numTriggers;

			/*AniNode* toNode = m_AniNodes[targetNodeIndex].get();
			auto currArrow = m_AniNodes[i].AddArrow(toNode);
			currArrow->aniEndIsChange = isAniEndChange;
			currArrow->type = TO_ANI_ARROW_TYPE(arrowType);
			currArrow->triggers.reserve(numTriggers);

			for (size_t z = 0; z < numTriggers; z++)
			{
				int triggerType = 0;
				int dataType = 0;

				load >> triggerType;
				load >> dataType;

				CGH::UnionData standard;
				standard.type = static_cast<CGH::DATA_TYPE>(dataType);

				switch (standard.type)
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
			}*/
		}
	}

	load.close();
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