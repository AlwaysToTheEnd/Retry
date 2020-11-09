#pragma once
#include "GameObject.h"
#include <Keyboard.h>
#include "AnimationTree.h"
#include "StaticObject.h"
#include "CGHScene.h"

class DOAnimator;
class UIPanel;
class UIParam;
class UIButton;
class AniArowVisual;
class DORenderer;
class DOUICollision;
class DOTransform;

namespace Ani
{
	class SkinnedData;
}

class AniNodeVisual :public GameObject
{
public:
	AniNodeVisual(CGHScene& scene, GameObject* parent, const char* typeName)
		: GameObject(scene, parent, typeName)
		, m_CurrSkinAnimationNames(nullptr)
		, m_CurrSkinAnimationEndTick(nullptr)
		, m_Panel(nullptr)
		, m_CurrAniNode(nullptr)
		, m_RoofControlButton(nullptr)
		, m_PanelCol(nullptr)
		, m_AniNameParam(nullptr)
	{

	}
	virtual ~AniNodeVisual() = default;

	void DeleteAniNode();
	void SetRenderValue(AniTree::AniNode* node, std::function<void()> excuteFunc, std::function<void()> deleteFunc);

	void SetSkinAnimationInfoVectorPtr(const std::vector<std::string>* aniNames, const std::vector<double>* aniEnds);

	physx::PxVec2			GetPos() const;
	const physx::PxVec2&	GetSize() const;

private:
	virtual void Init() override;
	virtual void Update(float delta) override;
	void ChangeAniRoof(AniTree::AniNode* node, UIButton* button);
	void ChangedTargetAni();

private:
	const std::vector<std::string>*			m_CurrSkinAnimationNames;
	const std::vector<double>*				m_CurrSkinAnimationEndTick;

	AniTree::AniNode*						m_CurrAniNode;
	std::string								m_CurrAniName;
	std::vector<AniArowVisual*>				m_Arrows;
	UIPanel*								m_Panel;
	UIButton*								m_RoofControlButton;
	UIParam*								m_AniNameParam;
	DOUICollision*							m_PanelCol;
	std::function<void()>					m_DeleteAninodeFunc;
};

class AniArowVisual :public GameObject
{
public:
	AniArowVisual(CGHScene& scene, GameObject* parent, const char* typeName)
		: GameObject(scene, parent, typeName)
		, m_IsIniting(false)
		, m_Transform(nullptr)
		, m_UICollison(nullptr)
		, m_Renderer(nullptr)
	{

	}
	virtual ~AniArowVisual() = default;

	void SetRenderValue(const physx::PxVec2& _from, const physx::PxVec2& _fromSize,
		const physx::PxVec2& _to, const physx::PxVec2& _toSize, std::function<void()> excuteFunc, bool isMousePos);

private:
	virtual void Init() override;
	virtual void Update(float delta) override;

private:
	bool				m_IsIniting;
	DOTransform*		m_Transform;
	DOUICollision*		m_UICollison;
	DORenderer*			m_Renderer;
};

class VisualizedAniTreeCreator :public GameObject
{
	struct NodePosData
	{
		physx::PxVec2 pos;
		physx::PxVec2 size;
	};

public:
	VisualizedAniTreeCreator(CGHScene& scene, GameObject* parent, const char* typeName)
		: GameObject(scene, parent, typeName)
		, m_CurrTree(nullptr)
		, m_AniTreeParam(nullptr)
		, m_CurrSkin(nullptr)
		, m_WorkPanel(nullptr)
		, m_ArrowAttributePanel(nullptr)
		, m_Animator(nullptr)
		, m_CurrInitingArrowIndex(-1)
	{

	}
	virtual ~VisualizedAniTreeCreator();
	virtual void Delete() override;
	
	void SelectSkinnedData(const std::string& name);

private:
	virtual void Init() override;
	virtual void Update(float delta) override;
	
	void AddNode();
	void AniNodeExcute(const AniTree::AniNode& node);
	void CancleExcute();
	
	void SetAnimationTreeListsParamToPanel(int posX, int posY, UIPanel* workPanel);
	void SetAniArrowAttributePanel(AniTree::AniArrow* arrow);
	void ChangeType(UIParam* target, CGH::UnionData* data);
	void AddParam(AniTree::AniArrow* arrow);
	void DeleteArrow(AniTree::AniArrow* arrow);

	void SelectNullTree();
	void ChangedTree();
	void SaveTree();

private:
	std::unique_ptr<AniTree::AnimationTree>			m_NullTree;
	AniTree::AnimationTree*							m_CurrTree;
	UIPanel*										m_WorkPanel;
	UIPanel*										m_ArrowAttributePanel;
	UIParam*										m_AniTreeParam;
	DOAnimator*										m_Animator;
	DORenderer*										m_Renderer;

	std::string										m_CurrTreeName;
	std::vector<std::string>						m_TreeNames;

	const Ani::SkinnedData*							m_CurrSkin;
	std::vector<std::string>						m_SkinNames;

	std::vector<std::string>						m_AniNames;
	std::vector<double>								m_AniEndTimes;

	std::vector<AniNodeVisual*>						m_AniNodeVs;
	std::vector<AniArowVisual*>						m_AniArowVs;

	std::unordered_map<unsigned int, NodePosData>	m_NodePosDatas;

	int												m_CurrInitingArrowIndex;
	physx::PxVec2									m_CurrMousePos;
};