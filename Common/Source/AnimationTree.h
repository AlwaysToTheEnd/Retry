#pragma once
#include <string>
#include <vector>
#include <assert.h>

namespace AniTree
{
	enum DATA_TYPE
	{
		TYPE_BOOL,
		TYPE_FLOAT,
		TYPE_INT,
		TYPE_UINT,
	};

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

		TRIGGER_TYPE_OFF_AFTER_CHECK = 1 << 8,
	};

	struct UnionData
	{
		union
		{
			bool	_b;
			float	_f;
			int		_i;
			unsigned int _u;
		};
	};

	class TriggerData
	{
	public:
		TriggerData(TRIGGER_TYPE type, DATA_TYPE dataType, UnionData standard);
		bool IsTriggerOK();

	private:
		template<typename T> bool CheckData(TRIGGER_TYPE triggerTYPE, T a, T b);
		TRIGGER_TYPE GetTriggerFuncType();

	public:
		const UnionData m_Standard;
		const TRIGGER_TYPE m_TriggerType;
		const DATA_TYPE m_DataType;
		UnionData m_Trigger;
	};

	enum TO_ANI_ARROW_TYPE
	{
		TO_ANI_NODE_TYPE_ONE_OK,
		TO_ANI_NODE_TYPE_ALL_OK,
		TO_ANI_NODE_TYPE_USING_PREV_TYPE,
	};

	struct NodeArrow
	{
		NodeArrow(unsigned int toNodeIndex);

		TO_ANI_ARROW_TYPE type;
		const unsigned int targetNodeIndex;
		bool AniEndIsChange;
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

		void Update(unsigned long long deltaTime);
		const std::string& GetAniName() const;
		void SetAniName(const std::string& name) { m_TargetAniName = name; }
		unsigned long long GetCurrTick() const { return m_CurrTick; }
		unsigned long long GetEndTick() const { return m_AniEndTime; }

		void ResetTick() { m_CurrTick = 0; }
		bool IsRoofAni() { return m_RoofAni; }

	private:
		const unsigned int	m_Index;
		const unsigned int	m_AniEndTime;
		const std::string	m_NodeName;
		std::string			m_TargetAniName;

		bool				m_RoofAni;
		unsigned long long	m_CurrTick;
	};

	struct OutputTrigger
	{
		OutputTrigger(const std::string& _from,const std::string& _to, TriggerData& _trigger)
			:trigger(_trigger)
			,from(_from)
			,to(_to)
		{

		}

		const std::string& from;
		const std::string& to;
		TriggerData& trigger;
	};

	class AnimationTree
	{
	public:
		AnimationTree()
			: m_CurrAniNodeIndex(0)
		{

		}

		bool Update(unsigned long long deltaTime);

		void GetTriggers(std::vector<OutputTrigger>& out);
		bool AddAniNode(const std::string& aniName, unsigned int aniClipEndTime, bool roof);
		bool AddArrow(const std::string& from, const std::string& to,
			TO_ANI_ARROW_TYPE arrowType, CHANGE_CONDITION_TYPE type, const TriggerData* trigger);
		bool AddArrow(unsigned int from, unsigned int to,
			TO_ANI_ARROW_TYPE arrowType, CHANGE_CONDITION_TYPE type, const TriggerData* trigger);
		std::string GetCurrAnimationName() const;
		unsigned long long GetCurrAnimationTick() const;
		int GetIndex(const std::string& aniName) const;

	private:
		bool CheckArrowTrigger(NodeArrow& arrow, std::vector<TriggerData>& triggers, 
			unsigned long long currTick, unsigned long long aniEndTick);
		void TriggerReset();

	private:
		unsigned int										m_CurrAniNodeIndex;
		std::vector<AniNode>								m_AniNodes;
		std::vector<std::vector<NodeArrow>>					m_Arrows;
		std::vector<std::vector<std::vector<TriggerData>>>	m_Triggers;
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