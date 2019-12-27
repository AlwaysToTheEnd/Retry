#pragma once
#include "GameObject.h"
#include "AnimationTree.h"
#include "StaticObject.h"
#include "CGHScene.h"

class ComAnimator;
class UIPanel;

class AniNodeVisual :public GameObject
{
	class AniTreeArowVisual :public GameObject
	{
	public:
		AniTreeArowVisual(CGHScene& scene)
			:GameObject(scene)
			, m_From(nullptr)
			, m_To(nullptr)
		{

		}
		virtual ~AniTreeArowVisual() = default;

		void SetFromNode(AniNodeVisual* from);
		void SetCurrMousePos(DirectX::XMFLOAT2 pos);
		void SetToNode(AniNodeVisual* to);
	private:
		virtual void Init() override;
		virtual void Update() override;

	private:
		std::string m_Name;
		AniNodeVisual* m_From;
		AniNodeVisual* m_To;
	};

	static class AnimationTreeArrowCreater : public StaticGameObjectController
	{
	public:
		AnimationTreeArrowCreater()
			:StaticGameObjectController(false)
			, m_CurrFrom(nullptr)
			, m_CurrArrow(nullptr)
		{

		}
		virtual ~AnimationTreeArrowCreater() = default;

		void Excute(AniNodeVisual* aniNode);

	private:
		virtual void WorkClear() override;
		virtual void Update() override;

	private:
		AniNodeVisual*						m_CurrFrom;
		AniNodeVisual::AniTreeArowVisual*	m_CurrArrow;

	} s_AnitreeArrowCreater;

public:
	AniNodeVisual(CGHScene& scene)
		:GameObject(scene)
	{

	}
	virtual ~AniNodeVisual() = default;

	void SetTargetAninode(AniTree::AniNode* aniNode) { m_TargetAninode = aniNode; }
private:
	virtual void Init() override;
	virtual void Update() override;

private:
	void AddArrow(AniTreeArowVisual* arrow);
	void StagingToArrowCreater();

private:
	std::vector<AniTreeArowVisual*> m_Arrows;
	UIPanel*			m_Panel;
	AniTree::AniNode*	m_TargetAninode;
};

