#include "GraphicComponent.h"
#include "BaseComponent.h"
#include "GameObject.h"

const std::unordered_map<std::string, MeshObject>* ComMesh::m_Meshs = nullptr;

std::vector<RenderInfo>* ComRenderer::m_ReservedRenderObjects = nullptr;

const std::unordered_map<std::string, Ani::SkinnedData>* ComAnimator::m_SkinnedDatas = nullptr;
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

void ComAnimator::Update()
{
	//#TODO Input Delta time
	m_CurrTick += 1;

	if (m_AniTree != nullptr)
	{
		if (m_AniTree->Update(10))
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

void ComAnimator::SetAnimationTree(bool value)
{
	if (value)
	{
		m_AniTree = std::make_unique<AniTree::AnimationTree>();
	}
	else
	{
		m_AniTree = nullptr;
	}
}

void ComAnimator::GetAniNames(std::vector<std::string>& out) const
{
	out.clear();

	if (m_CurrSkinnedData)
	{
		out = m_CurrSkinnedData->GetAnimationNames();
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

void ComRenderer::Update()
{
	auto comTransform = m_TargetGameObject.GetComponent<ComTransform>();

	assert(comTransform != nullptr);

	DirectX::XMMATRIX mat = DirectX::XMLoadFloat4x4(comTransform->GetMatrix());
	DirectX::XMStoreFloat4x4(m_RenderInfo.world, DirectX::XMMatrixTranspose(mat));

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
		assert(false);
		break;
	}

	m_ReservedRenderObjects->push_back(m_RenderInfo);
}

void ComRenderer::RenderMesh()
{
	auto comMesh = m_TargetGameObject.GetComponent<ComMesh>();
	auto comAnimator = m_TargetGameObject.GetComponent<ComAnimator>();

	if (comMesh != nullptr )
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

void ComFont::Update()
{
	//#TODO
	RenderFont addedFont(m_FontName, m_Text);
	addedFont.pos.x = m_OffsetPos.x;
	addedFont.pos.y = m_OffsetPos.y;
	addedFont.fontHeight = m_FontHeight;
	addedFont.color = m_Color;
	addedFont.drawSize = &m_DrawSize;

	m_ReservedFonts->push_back(addedFont);
}