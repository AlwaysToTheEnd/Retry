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
		TRIGGER_TYPE_UNKOWN,
		TRIGGER_TYPE_GRATER,
		TRIGGER_TYPE_LESS,
		TRIGGER_TYPE_SAME,

		TRIGGER_TYPE_DATA_ZEROSET_AFTER_CHECK = 1 << 6,
	};

	class TriggerData
	{
	public:
		TriggerData();
		TriggerData(TRIGGER_TYPE type, CGH::UnionData standard);
		int IsTriggerOK();

	private:
		template<typename T> bool CheckData(TRIGGER_TYPE triggerTYPE, T a, T b);
		TRIGGER_TYPE GetTriggerFuncType() const;

	public:
		TRIGGER_TYPE	m_TriggerType;
		CGH::UnionData	m_Standard;
		CGH::UnionData	m_Trigger;
	};

	enum TO_ANI_ARROW_TYPE
	{
		TO_ANI_NODE_TYPE_ONE_OK,
		TO_ANI_NODE_TYPE_ALL_OK,
		TO_ANI_NODE_TYPE_USING_PREV_TYPE,
	};

	struct AniArrow
	{
		TO_ANI_ARROW_TYPE			type = TO_ANI_NODE_TYPE_ONE_OK;
		bool						aniEndIsChange = false;
		std::string					nodeName;
		std::string					targetNodeName;
		std::vector<TriggerData>	triggers;
	};

	class AniNode
	{
	public:
		AniNode()
			: m_Pos(0,0)
			, m_AniEndTime(0)
			, m_CurrTime(0)
			, m_RoofAni(false)
		{

		}

		std::string Update(float deltaTime, std::vector<AniArrow>& arrows);

		const std::string& GetAniName() const;
		void SetAniName(const std::string& name, double aniEndTime);
		double GetCurrTime() const { return m_CurrTime; }
		double GetEndTime() const { return m_AniEndTime; }

		bool IsRoofAni() const { return m_RoofAni; }
		void SetRoofAni(bool value) { m_RoofAni = value; }

		void SetPos(physx::PxVec2 pos) { m_Pos = pos; }
		physx::PxVec2 GetPos() { return m_Pos; }

	private:
		bool CheckArrowTrigger(const AniArrow& arrow, std::vector<TriggerData>& triggers,
			double currTick, double aniEndTick);

	public:
		physx::PxVec2			m_Pos;
		double					m_AniEndTime;
		double					m_CurrTime;
		std::string				m_TargetAniName;
		std::string				m_NodeName;

		bool					m_RoofAni;
	};

	std::ostream& operator <<(std::ostream& os, const AniNode& node);

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

		std::vector<AniArrow>&			GetArrows() { return m_Arrows; }
		std::vector<AniNode>&			GetNodes() { return m_AniNodes; }
		unsigned int					GetNumNodes() { return CGH::SizeTTransUINT(m_AniNodes.size()); }
		double							GetCurrAnimationTime() const;
		std::string						GetCurrAnimationName() const;
		const std::string&				GetCurrSkinName() const { return m_CurrSkinName; }
		const std::string&				GetCurrMeshName() const { return m_CurrMeshName; }
		
		void AddAniNode();
		void AddArrow(const AniArrow& arrow) { m_Arrows.push_back(arrow); }
		void DeleteNode(const std::string& node);

		void SetCurrSkinName(const std::string& name) { m_CurrSkinName = name; }
		void SetCurrMeshName(const std::string& name) { m_CurrMeshName = name; }

	private:
		bool CheckArrowTrigger(AniArrow& arrow, std::vector<TriggerData>& triggers, 
			double currTick, double aniEndTick);
		int GetIndex(const std::string& nodeName);
		void TriggerReset();

	private:
		unsigned int			m_CurrAniNodeIndex;
		std::string				m_CurrSkinName;
		std::string				m_CurrMeshName;
		std::vector<AniNode>	m_AniNodes;
		std::vector<AniArrow>	m_Arrows;
	};

	template<typename T>
	inline bool TriggerData::CheckData(const TRIGGER_TYPE triggerTYPE,T a, T b)
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