#include "AnimationTreeCreator.h"
#include "GraphicComponent.h"

void AnimationTreeCreator::AniNodeViewObject::Init()
{

}

void AnimationTreeCreator::AniNodeViewObject::Update()
{

}


void AnimationTreeCreator::Init()
{
	m_Animator = AddComponent<ComAnimator>();
	m_Animator->GetSkinNames(m_SkinNames);

	m_CurrAniTree = std::make_unique<AniTree::AnimationTree>();
}

void AnimationTreeCreator::Update()
{

}

bool AnimationTreeCreator::LoadAniTreeFile(const std::wstring& filePath)
{
	return false;
}

