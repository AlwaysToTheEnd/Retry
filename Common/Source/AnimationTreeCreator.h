#pragma once
#include "GameObject.h"
#include "AnimationTree.h"

class ComAnimator;

class AnimationTreeCreator : public GameObject
{
private:
	class AniNodeViewObject :public GameObject
	{
	public:
		AniNodeViewObject() = default;
		virtual ~AniNodeViewObject() = default;

		virtual void Init() override;
		virtual void Update() override;
	private:

	};

public:
	AnimationTreeCreator() = default;
	virtual ~AnimationTreeCreator() = default;

	virtual void Init() override;
	virtual void Update() override;

	bool LoadAniTreeFile(const std::wstring& filePath);
	std::unique_ptr<AniTree::AnimationTree> GetAniTree() { return std::move(m_CurrAniTree); }

private:
	ComAnimator*					m_Animator = nullptr;
	std::vector<std::string>		m_SkinNames;
	std::vector<AniNodeViewObject>	m_ViewObjects;
	std::unique_ptr<AniTree::AnimationTree> m_CurrAniTree;
};

