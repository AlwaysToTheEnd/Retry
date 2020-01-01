#pragma once
#include <string>
#include <vector>
#include <assert.h>
#include "BaseClass.h"

namespace AniTree
{
	enum CHANGE_CONDITION_TYPE
	{
		CHANGE_CONDITION_TYPE_ANI_END,
		CHANGE_CONDITION_TYPE_TRIGGER
	};

	enum TRIGGER_TYPE
	{
		TRIGGER_TYPE_GRATER = 0,
		TRIGGER_TYPE_LESS = 0x1,
		TRIGGER_TYPE_SAME = 0x2,

		TRIGGER_TYPE_OFF_AFTER_CHECK = 1 << 6,
	};

	class TriggerData
	{
	public:
		TriggerData(TRIGGER_TYPE type, CGH::UnionData standard);
		bool IsTriggerOK();

	private:
		template<typename T> bool CheckData(TRIGGER_TYPE triggerTYPE, T a, T b);
		TRIGGER_TYPE GetTriggerFuncType();

	public:
		CGH::UnionData	m_Standard;
		TRIGGER_TYPE	m_TriggerType;
		CGH::UnionData	m_Trigger;
	};

	enum TO_ANI_ARROW_TYPE
	{
		TO_ANI_NODE_TYPE_ONE_OK,
		TO_ANI_NODE_TYPE_ALL_OK,
		TO_ANI_NODE_TYPE_USING_PREV_TYPE,
	};

	struct NodeArrow
	{
		NodeArrow(const std::string& toNodeName);

		TO_ANI_ARROW_TYPE type;
		std::string targetNode;
		bool aniEndIsChange;
		std::vector<TriggerData> triggers;
	};

	struct OutputArrow
	{
		OutputArrow(const std::string& _from, const std::string& _to, 
			std::vector<TriggerData>& _trigger, bool& endIsChange, TO_ANI_ARROW_TYPE& _type)
			:trigger(_trigger)
			, from(_from)
			, to(_to)
			, aniEndIsChange(endIsChange)
			, type(_type)
		{

		}

		bool&						aniEndIsChange;
		TO_ANI_ARROW_TYPE&			type;
		std::string					from;
		std::string					to;
		std::vector<TriggerData>&	trigger;
	};

	class AniNode
	{
	public:
		AniNode(const std::string& ani, unsigned int aniClipEndTime, bool roof, unsigned int index)
			: m_NodeName(ani)
			, m_Index(index)
			, m_AniEndTime(aniClipEndTime)
			, m_CurrTick(0)
			, m_RoofAni(roof)
		{

		}

		std::string Update(unsigned long long deltaTime);
		const std::string& GetAniName() const;
		const std::string& GetNodeName() const { return m_NodeName; }
		void SetAniName(const std::string& name) { m_TargetAniName = name; }
		unsigned long long GetCurrTick() const { return m_CurrTick; }
		unsigned long long GetEndTick() const { return m_AniEndTime; }
		void GetArrows(std::vector<OutputArrow>& out, const std::string& to="");
		const std::vector<NodeArrow>& GetArrows() { return m_Arrows; }

		void AddArrow(const std::string& to);
		bool AddTrigger(const std::string& to, TO_ANI_ARROW_TYPE arrowType, CHANGE_CONDITION_TYPE type, const TriggerData* trigger);

		void TriggerReset();
		bool IsRoofAni() const { return m_RoofAni; }

	private:
		bool CheckArrowTrigger(NodeArrow& arrow, std::vector<TriggerData>& triggers,
			unsigned long long currTick, unsigned long long aniEndTick);

	private:
		const unsigned int	m_Index;
		const unsigned int	m_AniEndTime;
		const std::string	m_NodeName;
		std::string			m_TargetAniName;

		bool					m_RoofAni;
		unsigned long long		m_CurrTick;
		std::vector<NodeArrow>	m_Arrows;
	};

	class AnimationTree final
	{
	public:
		AnimationTree()
			: m_CurrAniNodeIndex(0)
		{

		}

		bool Update(unsigned long long deltaTime);

		void GetArrows(std::vector<OutputArrow>& out);
		bool AddAniNode(const std::string& aniName, unsigned int aniClipEndTime, bool roof);
		AniNode* GetAniNode(const std::string aniName);
		bool AddTrigger(const std::string& from, const std::string& to,
			TO_ANI_ARROW_TYPE arrowType, CHANGE_CONDITION_TYPE type, const TriggerData* trigger);
		bool AddTrigger(unsigned int from, unsigned int to,
			TO_ANI_ARROW_TYPE arrowType, CHANGE_CONDITION_TYPE type, const TriggerData* trigger);
		std::string GetCurrAnimationName() const;
		unsigned long long GetCurrAnimationTick() const;
		int GetIndex(const std::string& aniName) const;

	private:
		bool CheckArrowTrigger(NodeArrow& arrow, std::vector<TriggerData>& triggers, 
			unsigned long long currTick, unsigned long long aniEndTick);

	private:
		unsigned int										m_CurrAniNodeIndex;
		std::vector<AniNode>								m_AniNodes;
	};


	template<typename T>
	inline bool TriggerData::CheckData(TRIGGER_TYPE triggerTYPE, T a, T b)
	{
		switch (triggerTYPE)
		{
		case AniTree::TRIGGER_TYPE_GRATER:
			return a > b;
			break;
		case AniTree::TRIGGER_TYPE_LESS:
			return a < b;
			break;
		case AniTree::TRIGGER_TYPE_SAME:
			return a == b;
			break;
		default:
			break;
		}

		return false;
	}
}