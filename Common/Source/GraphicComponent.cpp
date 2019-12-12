#include "GraphicComponent.h"
#include "BaseComponent.h"
#include "GameObject.h"

const std::unordered_map<std::string, MeshObject>* ComMesh::m_Meshs=nullptr;

std::vector<RenderInfo>* ComRenderer::m_ReservedRenderObjects = nullptr;

const std::unordered_map<std::string, Ani::SkinnedData>* ComAnimator::m_SkinnedDatas = nullptr;
std::vector<AniBoneMat>* ComAnimator::m_ReservedAniBone = nullptr;

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
	if (m_CurrSkinnedData && m_CurrAniName.length())
	{
		m_CurrTick++;
		//#TODO: Get deltaTime and add to currTick
		m_BoneMatStoredIndex = m_ReservedAniBone->size();
		m_ReservedAniBone->emplace_back();
		m_CurrSkinnedData->GetFinalTransforms(m_CurrAniName, m_CurrTick, m_ReservedAniBone->back());

		if (m_IsRoof)
		{
			if (m_CurrTick > m_CurrSkinnedData->GetClipEndTime(m_CurrAniName))
			{
				m_CurrTick = 0;
			}
		}
	}
	else
	{
		m_BoneMatStoredIndex = -1;
	}
}

void ComAnimator::GetSkinNames(std::vector<std::string>& out)
{
	for (auto& it : *m_SkinnedDatas)
	{
		out.push_back(it.first);
	}
}

void ComAnimator::GetAniNames(std::vector<std::string>& out)
{
	if (m_CurrSkinnedData)
	{
		out = m_CurrSkinnedData->GetAnimationNames();
	}
}

bool ComAnimator::SelectSkin(const std::string& name)
{
	auto iter = m_SkinnedDatas->find(name);
	m_CurrAniName.clear();
	m_CurrTick = 0;

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
	if (m_CurrSkinnedData==nullptr)
	{
		return false;
	}
	
	if (m_CurrSkinnedData->CheckAnimation(name))
	{
		m_CurrAniName = name;
		return true;
	}
}

void ComRenderer::Update()
{
	RenderInfo addInfo;
	auto comTransform = m_TargetGameObject->GetComponent<ComTransform>();
	auto comMesh = m_TargetGameObject->GetComponent<ComMesh>();
	auto comAnimator = m_TargetGameObject->GetComponent<ComAnimator>();

	if (comMesh != nullptr && comTransform != nullptr)
	{
		addInfo.meshName = comMesh->GetCurrMeshName();
		addInfo.world = comTransform->GetMatrix();
	}
	else
	{
		assert(false);
	}

	if (comAnimator != nullptr)
	{
		addInfo.aniBoneIndex = comAnimator->GetBoneMatStoredIndex();
	}

	m_ReservedRenderObjects->push_back(addInfo);
}