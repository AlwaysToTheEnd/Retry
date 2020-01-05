#include "GraphicComponent.h"
#include "BaseComponent.h"
#include "GameObject.h"

const std::unordered_map<std::string, MeshObject>* ComMesh::m_Meshs = nullptr;

std::vector<RenderInfo>* ComRenderer::m_ReservedRenderObjects = nullptr;

const std::unordered_map<std::string, Ani::SkinnedData>* ComAnimator::m_SkinnedDatas = nullptr;
std::unordered_map<std::wstring, std::unique_ptr<AniTree::AnimationTree>>* ComAnimator::m_AnimationTrees = nullptr;
std::vector<AniBoneMat>* ComAnimator::m_ReservedAniBone = nullptr;

std::vector<RenderFont>* ComFont::m_ReservedFonts = nullptr;

void ComMesh::GetMeshNames(std::vector<std::string>& out)
{
	for (auto& it : *m_Meshs)
	{
		out.push_back(it.first);
	}
}

bool ComMesh::SelectMesh(const std::string& name)
{
	auto iter = m_Meshs->find(name);
	if (iter == m_Meshs->end())
	{
		m_CurrMesh = nullptr;
		m_CurrMeshName.clear();
		return false;
	}

	m_CurrMesh = &iter->second;
	m_CurrMeshName = name;
	return true;
}

void ComAnimator::Update(float delta)
{
	m_CurrTick += delta;

	if (m_AniTree != nullptr)
	{
		if (m_AniTree->Update(delta))
		{
			m_CurrTick = m_AniTree->GetCurrAnimationTick();
			m_CurrAniName = m_AniTree->GetCurrAnimationName();
		}
	}

	if (m_CurrSkinnedData && m_CurrAniName.length())
	{
		m_BoneMatStoredIndex = m_ReservedAniBone->size();
		m_ReservedAniBone->emplace_back();
		m_CurrSkinnedData->GetFinalTransforms(m_CurrAniName, m_CurrTick, m_ReservedAniBone->back());
	}
	else
	{
		m_BoneMatStoredIndex = -1;
	}
}

void ComAnimator::GetSkinNames(std::vector<std::string>& out)
{
	assert(m_SkinnedDatas);
	out.clear();

	for (auto& it : *m_SkinnedDatas)
	{
		out.push_back(it.first);
	}
}

void ComAnimator::SetAnimationTree(AniTree::AnimationTree* tree)
{
	m_AniTree = tree;
}

void ComAnimator::GetAniNames(std::vector<std::string>& out) const
{
	if (m_CurrSkinnedData)
	{
		m_CurrSkinnedData->GetAnimationNames(out);
	}
}

bool ComAnimator::SelectSkin(const std::string& name)
{
	auto iter = m_SkinnedDatas->find(name);
	m_CurrAniName.clear();

	if (iter == m_SkinnedDatas->end())
	{
		m_CurrSkinnedData = nullptr;
		return false;
	}

	m_CurrSkinnedData = &iter->second;
	return true;
}

bool ComAnimator::SelectAnimation(const std::string& name)
{
	if (m_CurrSkinnedData != nullptr)
	{
		if (m_CurrSkinnedData->CheckAnimation(name))
		{
			m_CurrAniName = name;
			return true;
		}
	}

	return false;
}

void ComRenderer::Update(float delta)
{
	auto comTransform = m_TargetGameObject.GetComponent<ComTransform>();

	assert(comTransform != nullptr);

	m_RenderInfo.world = comTransform->GetMatrix().getTranspose();

	switch (m_RenderInfo.type)
	{
	case RENDER_MESH:
	{
		RenderMesh();
	}
	break;
	case RENDER_BOX:
	case RENDER_PLANE:
	case RENDER_TEX_PLANE:
	case RENDER_UI:
		break;
	default:
		return;
		//assert(false);
		break;
	}

	m_ReservedRenderObjects->push_back(m_RenderInfo);
}

void ComRenderer::RenderMesh()
{
	auto comMesh = m_TargetGameObject.GetComponent<ComMesh>();
	auto comAnimator = m_TargetGameObject.GetComponent<ComAnimator>();

	if (comMesh != nullptr)
	{
		m_RenderInfo.meshOrTextureName = comMesh->GetCurrMeshName();
	}
	else
	{
		assert(false);
	}

	if (comAnimator != nullptr)
	{
		m_RenderInfo.mesh.aniBoneIndex = comAnimator->GetBoneMatStoredIndex();
	}
}

void ComFont::Update(float delta)
{
	//#TODO
	if (m_Text.size())
	{
		RenderFont addedFont(m_FontName, m_Text);
		addedFont.pos = m_Pos;
		addedFont.fontHeight = m_FontHeight;
		addedFont.color = m_Color;
		addedFont.drawSize = &m_DrawSize;
		addedFont.benchmark = m_Benchmark;

		m_ReservedFonts->push_back(addedFont);
	}
}