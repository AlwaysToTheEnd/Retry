#pragma once
#include <string>
#include <vector>
#include <assert.h>
#include <memory>
#include <fstream>
#include <functional>
#include "BaseClass.h"

namespace AniTree
{
	class AniNode;

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
		void SetAniName(const std::string& name, unsigned int aniEndTime);
		unsigned long long GetCurrTick() const { return m_CurrTick; }
		unsigned long long GetEndTick() const { return m_AniEndTime; }
		void GetArrows(std::vector<OutputArrow>& out, const AniNode* to=nullptr);
		const std::vector<NodeArrow>& GetArrows() { return m_Arrows; }

		AniTree::NodeArrow* AddArrow(const AniNode* to);
		void DeleteArrow(const AniNode* to);
		void DeleteTrigger(const AniNode* to, int index);

		void TriggerReset();

		bool IsRoofAni() const { return m_RoofAni; }
		void SetRoofAni(bool value) { m_RoofAni = value; }

	public:
		void SetIndexFunc(std::function<int()> func) { m_IndexFunc = func; }
		friend std::ostream& operator <<(std::ostream& os, const AniNode& node);

	private:
		bool CheckArrowTrigger(NodeArrow& arrow, std::vector<TriggerData>& triggers,
			unsigned long long currTick, unsigned long long aniEndTick);

	private:
		unsigned int		m_AniEndTime;
		std::string			m_TargetAniName;

		bool					m_RoofAni;
		unsigned long long		m_CurrTick;
		std::vector<NodeArrow>	m_Arrows;

		std::function<int()>	m_IndexFunc;
	};

	class AnimationTree final
	{
	public:
		AnimationTree()
			: m_CurrAniNodeIndex(0)
		{

		}

		bool Update(float deltaTime);


		void SaveTree(const std::wstring& fileFath);
		void LoadTree(const std::wstring& fileFath);

		void				GetArrows(std::vector<OutputArrow>& out);
		unsigned long long	GetCurrAnimationTick() const;
		std::string			GetCurrAnimationName() const;
		const std::string&	GetCurrSkinName() const { return m_CurrSkinName; }
		const std::string&	GetCurrMeshName() const { return m_CurrMeshName; }
		
		AniNode*	AddAniNode();
		void		DeleteNode(const AniNode* node);

		void SetCurrSkinName(const std::string& name) { m_CurrSkinName = name; }
		void SetCurrMeshName(const std::string& name) { m_CurrMeshName = name; }

	private:
		bool CheckArrowTrigger(NodeArrow& arrow, std::vector<TriggerData>& triggers, 
			unsigned long long currTick, unsigned long long aniEndTick);
		int GetIndex(const AniNode* node);

	private:
		unsigned int										m_CurrAniNodeIndex;
		std::string											m_CurrSkinName;
		std::string											m_CurrMeshName;
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