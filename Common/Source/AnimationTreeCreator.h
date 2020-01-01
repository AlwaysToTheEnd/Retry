#pragma once
#include "GameObject.h"
#include <Keyboard.h>
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
	static class AnimationTreeArrowCreator : public StaticGameObjectController
	{
	public:
		AnimationTreeArrowCreator()
			:StaticGameObjectController(false)
			, m_CurrFrom(nullptr)
			, m_CurrArrow(nullptr)
			, m_isNextClear(false)
		{

		}
		virtual ~AnimationTreeArrowCreator() = default;

		void Excute(AniNodeVisual* aniNode);

	private:
		virtual void WorkClear() override;
		virtual void Update(unsigned long long delta) override;

	private:
		AniNodeVisual*		m_CurrFrom;
		AniTreeArowVisual*	m_CurrArrow;
		bool				m_isNextClear;

	} s_AnitreeArrowCreater;

public:
	AniNodeVisual(CGHScene& scene)
		:GameObject(scene)
		,m_Panel(nullptr)
		,m_GetTargetAninodeFunc(nullptr)
	{

	}
	virtual ~AniNodeVisual() = default;
	virtual void Delete() override;

	void ArrowVisualDeleted(AniTreeArowVisual* arrow);
	void SetTargetAninodeFunc(std::function<AniTree::AniNode*(void)> func);
	AniTree::AniNode* GetNode() { return m_GetTargetAninodeFunc(); }
	const std::string& GetNodeName() const { return m_GetTargetAninodeFunc()->GetNodeName(); }
	const DirectX::XMFLOAT2& GetPos() const;
	const DirectX::XMFLOAT3& GetSize() const;

private:
	virtual void Init() override;
	virtual void Update(unsigned long long delta) override;

private:
	void AddArrow(AniTreeArowVisual* arrow);

private:
	std::vector<AniTreeArowVisual*>	m_Arrows;
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
		virtual void Update(unsigned long long delta) override;
		void CreateAttributePanel();
		void AddParam();
		void KeyboardWork(const DirectX::Keyboard::KeyboardStateTracker* keyboard);

	private:
		const static int					m_FontSize = 12;

		AniTreeArowVisual*					m_CurrArrow;
		UIPanel*							m_AttributePanel;

		AniTree::TO_ANI_ARROW_TYPE			m_ArrowType;
		AniTree::TRIGGER_TYPE				m_TriggerType;
		CGH::UnionData						m_Standard;
		std::vector<AniTree::OutputArrow>	m_Arrows;
		
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
	void SetToNode(AniNodeVisual* to);
	void SetCurrMousePos(DirectX::XMFLOAT2 pos) { m_MousePos = pos; }
	const AniNodeVisual* GetToNode() const { return m_To; }

private:
	virtual void Init() override;
	virtual void Update(unsigned long long delta) override;

private:
	std::string			m_Name;
	ComTransform*		m_Transform;
	ComUICollision*		m_UICollison;
	ComRenderer*		m_Renderer;
	AniNodeVisual*		m_From;
	AniNodeVisual*		m_To;
	DirectX::XMFLOAT2	m_MousePos;
};

class VisualizedAniTreeCreator :public GameObject
{
public:
	VisualizedAniTreeCreator(CGHScene& scene)
		: GameObject(scene)
		, m_CurrSkin(nullptr)
		, m_WorkPanel(nullptr)
		, m_Animator(nullptr)
	{

	}

	void SelectSkinnedData(const std::string& name);

private:
	virtual void Init() override;
	virtual void Update(unsigned long long delta) override;
	void AddNode(int aniIndex);

private:
	std::unique_ptr<AniTree::AnimationTree>	m_Tree;
	UIPanel*								m_WorkPanel;
	ComAnimator*							m_Animator;
	const Ani::SkinnedData*					m_CurrSkin;
	std::vector<std::string>				m_AniNames;
};