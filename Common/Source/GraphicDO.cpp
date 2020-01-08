#include "GraphicDO.h"
#include "PhysicsDO.h"
#include "GameObject.h"
#include "d3dApp.h"

const std::unordered_map<std::string, MeshObject>* DORenderMesh::m_Meshs = nullptr;

std::vector<RenderInfo>* DORenderer::m_ReservedRenderObjects = nullptr;

const std::unordered_map<std::string, Ani::SkinnedData>* DOAnimator::m_SkinnedDatas = nullptr;
std::unordered_map<std::string, std::unique_ptr<AniTree::AnimationTree>>* DOAnimator::m_AnimationTrees = nullptr;
std::vector<AniBoneMat>* DOAnimator::m_ReservedAniBone = nullptr;

std::vector<RenderFont>* DOFont::m_ReservedRenderFonts = nullptr;

void DORenderMesh::GetMeshNames(std::vector<std::string>& out)
{
	for (auto& it : *m_Meshs)
	{
		out.push_back(it.first);
	}
}

bool DORenderMesh::SelectMesh(const std::string& name)
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

void DORenderMesh::Init(PhysX4_1*, IGraphicDevice* graphicDevice)
{
	if (m_Meshs == nullptr)
	{
		m_Meshs = graphicDevice->GetMeshDataMap();
	}
}

void DOAnimator::Update(float delta)
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

void DOAnimator::Init(PhysX4_1*, IGraphicDevice* graphicDevice)
{
	if (m_ReservedAniBone == nullptr)
	{
		m_ReservedAniBone = graphicDevice->GetReservedAniBoneMatVector();
		m_AnimationTrees = graphicDevice->GetAnimationTreeMap();
		m_SkinnedDatas = graphicDevice->GetSkinnedDataMap();
	}
}

void DOAnimator::GetSkinNames(std::vector<std::string>& out) const
{
	assert(m_SkinnedDatas);
	out.clear();

	for (auto& it : *m_SkinnedDatas)
	{
		out.push_back(it.first);
	}
}

bool DOAnimator::IsRegisteredTree(const AniTree::AnimationTree* tree) const
{
	for (auto& it : *m_AnimationTrees)
	{
		if (it.second.get() == tree)
		{
			return true;
		}
	}

	return false;
}

void DOAnimator::SetAnimationTree(AniTree::AnimationTree* tree)
{
	m_AniTree = tree;
}

void DOAnimator::SetAnimationTree(const std::string& fileName)
{
	auto iter = m_AnimationTrees->find(fileName);

	if (iter != m_AnimationTrees->end())
	{
		m_AniTree = iter->second.get();
	}
}

void DOAnimator::GetAnimationTreeNames(std::vector<std::string>& out) const
{
	out.clear();
	for (auto& it : *m_AnimationTrees)
	{
		out.push_back(it.first);
	}
}

void DOAnimator::SaveAnimationTree(const std::wstring& filePath, const std::string& fileName, AniTree::AnimationTree* tree)
{
	auto iter = m_AnimationTrees->find(fileName);

	if (iter == m_AnimationTrees->end())
	{
		m_AnimationTrees->insert({ fileName, std::unique_ptr<AniTree::AnimationTree>(tree) });
	}
	else
	{
		assert(iter->second.get() == tree);
	}

	tree->SaveTree(filePath);
}

void DOAnimator::SaveAnimationTree(const std::wstring& filePath, const std::string& fileName, std::unique_ptr<AniTree::AnimationTree> tree)
{
	auto iter = m_AnimationTrees->find(fileName);

	if (iter == m_AnimationTrees->end())
	{
		tree->SaveTree(filePath);
		m_AnimationTrees->insert({ fileName, std::move(tree) });
	}
	else
	{
		assert(iter->second.get() == tree.get());
	}
}

void DOAnimator::GetAniNames(std::vector<std::string>& out) const
{
	if (m_CurrSkinnedData)
	{
		m_CurrSkinnedData->GetAnimationNames(out);
	}
}

bool DOAnimator::SelectSkin(const std::string& name)
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

bool DOAnimator::SelectAnimation(const std::string& name)
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

void DORenderer::Update(float delta)
{
	auto comTransform = m_Parent->GetComponent<DOTransform>();

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

void DORenderer::Init(PhysX4_1*, IGraphicDevice* graphicDevice)
{
	if (m_ReservedRenderObjects == nullptr)
	{
		m_ReservedRenderObjects = graphicDevice->GetReservedRenderInfoVector();
	}
}

void DORenderer::RenderMesh()
{
	auto comMesh = m_Parent->GetComponent<DORenderMesh>();
	auto comAnimator = m_Parent->GetComponent<DOAnimator>();

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

void DOFont::Update(float delta)
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

		m_ReservedRenderFonts->push_back(addedFont);
	}
}

void DOFont::Init(PhysX4_1*, IGraphicDevice* graphicDevice)
{
	if (m_ReservedRenderFonts == nullptr)
	{
		m_ReservedRenderFonts = graphicDevice->GetReservedRenderFontVector();
	}
}
