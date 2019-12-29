#pragma once
#include "GameObject.h"
#include "AnimationTree.h"
#include "StaticObject.h"
#include "CGHScene.h"

class ComAnimator;
class UIPanel;
class AniTreeArowVisual;
class ComRenderer;
class ComUICollision;
class ComTransform;

namespace Ani
{
	class SkinnedData;
}

class AniNodeVisual :public GameObject
{
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
		AniNodeVisual*		m_CurrFrom;
		AniTreeArowVisual*	m_CurrArrow;

	} s_AnitreeArrowCreater;

public:
	AniNodeVisual(CGHScene& scene)
		:GameObject(scene)
		,m_OutputPos(0,0)
		,m_InputPos(0,0)
		,m_Panel(nullptr)
		,m_GetTargetAninodeFunc(nullptr)
	{

	}
	virtual ~AniNodeVisual() = default;
	virtual void Delete() override;

	void ArrowVisualDeleted(AniTreeArowVisual* arrow);
	void SetTargetAninodeFunc(std::function<AniTree::AniNode*(void)> func);
	const std::string& GetNodeName() const { return m_GetTargetAninodeFunc()->GetNodeName(); }
	const DirectX::XMFLOAT2& GetOutputPos() { return m_OutputPos; }
	const DirectX::XMFLOAT2& GetInputPos() { return m_InputPos; }

private:
	virtual void Init() override;
	virtual void Update() override;

private:
	void AddArrow(AniTreeArowVisual* arrow);

private:
	std::vector<AniTreeArowVisual*>	m_Arrows;
	DirectX::XMFLOAT2				m_OutputPos;
	DirectX::XMFLOAT2				m_InputPos;
	UIPanel*						m_Panel;
	std::function<AniTree::AniNode*(void)> m_GetTargetAninodeFunc;
};

class AniTreeArowVisual :public GameObject
{
private:
	static class AniTreeArrowArttributeEditer : public StaticGameObjectController
	{
	public:
		AniTreeArrowArttributeEditer()
			:StaticGameObjectController(false)
			, m_CurrArrow(nullptr)
			, m_AttributePanel(nullptr)
		{

		}
		virtual ~AniTreeArrowArttributeEditer() = default;

		void SetArrowVisual(AniTreeArowVisual* arrow);

	private:
		virtual void WorkClear() override;
		virtual void Update() override;

	private:
		AniTreeArowVisual*	m_CurrArrow;
		UIPanel*			m_AttributePanel;

		AniTree::TO_ANI_ARROW_TYPE m_ArrowType;
		AniTree::TRIGGER_TYPE m_TriggerType;
		CGH::UnionData m_Standard;
		
	} s_ArrowArttributeEditer;

public:
	AniTreeArowVisual(CGHScene& scene)
		:GameObject(scene)
		, m_From(nullptr)
		, m_To(nullptr)
		, m_MousePos(0,0)
	{

	}
	virtual ~AniTreeArowVisual() = default;
	virtual void Delete() override;

	void SetFromNode(AniNodeVisual* from) { m_From = from; }
	void SetToNode(AniNodeVisual* to) { m_To = to; }
	void SetCurrMousePos(DirectX::XMFLOAT2 pos) { m_MousePos = pos; }
	const AniNodeVisual* GetToNode() const { return m_To; }

private:
	virtual void Init() override;
	virtual void Update() override;

private:
	std::string			m_Name;
	ComTransform*		m_Transform;
	ComUICollision*		m_UICollison;
	ComRenderer*		m_Renderer;
	AniNodeVisual*		m_From;
	AniNodeVisual*		m_To;
	DirectX::XMFLOAT2	m_MousePos;
};

class VisualizedAniTreeCreater :public GameObject
{
public:
	VisualizedAniTreeCreater(CGHScene& scene)
		: GameObject(scene)
		, m_CurrSkin(nullptr)
		, m_WorkPanel(nullptr)
		, m_Animator(nullptr)
	{

	}

	void SelectSkinnedData(const std::string& name);

private:
	virtual void Init() override;
	virtual void Update() override;
	void AddNode(int aniIndex);

private:
	std::unique_ptr<AniTree::AnimationTree>	m_Tree;
	UIPanel*								m_WorkPanel;
	ComAnimator*							m_Animator;
	const Ani::SkinnedData*					m_CurrSkin;
	std::vector<std::string>				m_AniNames;
};