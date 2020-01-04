#pragma once
#include <string>
#include <vector>
#include <assert.h>
#include <memory>
#include "BaseClass.h"

namespace AniTree
{
	class AniNode;

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
		NodeArrow(const AniNode*);

		TO_ANI_ARROW_TYPE type;
		const AniNode* targetNode;
		bool aniEndIsChange;
		std::vector<TriggerData> triggers;
	};

	struct OutputArrow
	{
		OutputArrow(const AniNode* _from, const AniNode* _to,
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
		const AniNode*				from;
		const AniNode*				to;
		std::vector<TriggerData>&	trigger;
	};

	class AniNode
	{
	public:
		AniNode()
			: m_AniEndTime(0)
			, m_CurrTick(0)
			, m_RoofAni(false)
		{

		}

		const AniNode* Update(float deltaTime);
		const std::string& GetAniName() const;
		const std::string& GetNodeName() const { return m_NodeName; }
		void SetAniName(const std::string& name, unsigned int aniEndTime);
		unsigned long long GetCurrTick() const { return m_CurrTick; }
		unsigned long long GetEndTick() const { return m_AniEndTime; }
		void GetArrows(std::vector<OutputArrow>& out, const AniNode* to=nullptr);
		const std::vector<NodeArrow>& GetArrows() { return m_Arrows; }

		void AddArrow(const AniNode* to);
		void DeleteArrow(const AniNode* to);
		void DeleteTrigger(const AniNode* to, int index);
		bool AddTrigger(const AniNode* to, TO_ANI_ARROW_TYPE arrowType, CHANGE_CONDITION_TYPE type, const TriggerData* trigger);

		void TriggerReset();

		bool IsRoofAni() const { return m_RoofAni; }
		void SetRoofAni(bool value) { m_RoofAni = value; }

	private:
		bool CheckArrowTrigger(NodeArrow& arrow, std::vector<TriggerData>& triggers,
			unsigned long long currTick, unsigned long long aniEndTick);

	private:
		unsigned int		m_AniEndTime;
		std::string			m_NodeName;
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

		bool Update(float deltaTime);

		void GetArrows(std::vector<OutputArrow>& out);
		AniNode* AddAniNode();
		bool AddTrigger(AniNode* from, const AniNode* to,
			TO_ANI_ARROW_TYPE arrowType, CHANGE_CONDITION_TYPE type, const TriggerData* trigger);
		std::string GetCurrAnimationName() const;
		unsigned long long GetCurrAnimationTick() const;
		void DeleteNode(const AniNode* node);

	private:
		bool CheckArrowTrigger(NodeArrow& arrow, std::vector<TriggerData>& triggers, 
			unsigned long long currTick, unsigned long long aniEndTick);
		int GetIndex(const AniNode* node);

	private:
		unsigned int										m_CurrAniNodeIndex;
		std::vector<std::unique_ptr<AniNode>>				m_AniNodes;
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