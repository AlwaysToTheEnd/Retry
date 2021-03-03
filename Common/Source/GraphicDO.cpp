#include "GraphicDO.h"
#include "PhysicsDO.h"
#include "GameObject.h"
#include "d3dApp.h"

const std::unordered_map<std::string, MeshObject>* DORenderMesh::m_Meshs[CGH::MESH_TYPE_COUNT] = {};
std::function<void(const std::string &, physx::PxVec3)>	DORenderMesh::m_ReComputeHeightFieldFunc = nullptr;

std::vector<RenderInfo>* DORenderer::m_ReservedRenderObjects = nullptr;

const std::unordered_map<std::string, Ani::SkinnedData>* DOAnimator::m_SkinnedDatas = nullptr;
std::unordered_map<std::string, std::unique_ptr<AniTree::AnimationTree>>* DOAnimator::m_AnimationTrees = nullptr;
std::vector<AniBoneMat>* DOAnimator::m_ReservedAniBone = nullptr;

std::vector<RenderFont>* DOFont::m_ReservedRenderFonts = nullptr;

void DORenderMesh::GetMeshNames(CGH::MESH_TYPE type, std::vector<std::string>& out)
{
	for (auto& it : *m_Meshs[type])
	{
		out.push_back(it.first);
	}
}

bool DORenderMesh::SelectMesh(CGH::MESH_TYPE type, const std::string& name)
{
	auto iter = m_Meshs[type]->find(name);
	if (iter == m_Meshs[type]->end())
	{
		m_CurrMesh = nullptr;
		m_CurrMeshName.clear();
		return false;
	}

	m_CurrMesh = &iter->second;
	m_CurrMeshName = name;
	m_CurrMeshType = type;
	return true;
}

void DORenderMesh::ReComputeHeightField(physx::PxVec3 scale)
{
	assert(m_CurrMeshType == CGH::HEIGHTFIELD_MESH);

	m_ReComputeHeightFieldFunc(m_CurrMeshName, scale);
}

void DORenderMesh::Init(PhysX4_1*, IGraphicDevice* graphicDevice)
{
	if (m_Meshs[0] == nullptr)
	{
		for (int i = 0; i < CGH::MESH_TYPE_COUNT; i++)
		{
			m_Meshs[i] = graphicDevice->GetMeshDataMap(static_cast<CGH::MESH_TYPE>(i));
		}

		m_ReComputeHeightFieldFunc = std::bind(&IGraphicDevice::ReComputeHeightField, graphicDevice, std::placeholders::_1, std::placeholders::_2);
	}
}

void DOAnimator::Update(float delta)
{
	m_CurrTime += delta;

	if (m_AniTree != nullptr)
	{
		if (m_AniTree->Update(delta))
		{
			m_CurrTime = m_AniTree->GetCurrAnimationTime();
			m_CurrAniName = m_AniTree->GetCurrAnimationName();
		}
	}
	
	if (m_CurrSkinnedData && m_CurrAniName.length())
	{
		m_BoneMatStoredIndex = CGH::SizeTTransINT(m_ReservedAniBone->size());
		m_ReservedAniBone->emplace_back();
		m_CurrSkinnedData->GetFinalTransforms(m_CurrAniName, m_CurrTime, m_ReservedAniBone->back());
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

void DORenderer::DoFuncFromMouse(DirectX::Mouse::ButtonStateTracker::ButtonState state, DirectX::MOUSEBUTTONINDEX index)
{
	if (m_Funcs)
	{
		auto& indices = m_Funcs->buttonIndices;
		auto& states = m_Funcs->states;

		for (size_t i = 0; i < m_Funcs->funcs.size(); i++)
		{
			if (indices[i] == index && states[i] == state)
			{
				m_Funcs->funcs[i]();
			}
		}
	}
}

void DORenderer::AddPixelFunc(std::function<void()> func,
	DirectX::Mouse::ButtonStateTracker::ButtonState state, DirectX::MOUSEBUTTONINDEX index)
{
	if (m_Funcs==nullptr)
	{
		m_Funcs = std::make_unique<RenderFuncFromMouse>();
	}
	
	m_Funcs->funcs.push_back(func);
	m_Funcs->states.push_back(state);
	m_Funcs->buttonIndices.push_back(index);
}

void DORenderer::Update(float delta)
{
	auto comTransform = m_Parent->GetComponent<DOTransform>();

	assert(comTransform != nullptr);

	switch (m_RenderInfo.type)
	{
	case RENDER_SKIN:
	case RENDER_MESH:
	case RENDER_HEIGHT_FIELD:
	{
		m_RenderInfo.world = comTransform->GetRTMatrix().getTranspose();
		m_RenderInfo.scale = comTransform->GetScale();
		RenderMesh();
	}
	break;
	case RENDER_BOX:
	case RENDER_PLANE:
	case RENDER_2DPLANE:
		m_RenderInfo.world = comTransform->GetRTMatrix().getTranspose();
		m_RenderInfo.scale = comTransform->GetScale();
		break;
	case RENDER_LIGHT:
	case RENDER_UI:
		m_RenderInfo.world = comTransform->GetRTMatrix();
		break;
	default:
		return;
		//assert(false);
		break;
	}

	m_RenderInfo.pixelColID = GetDeviceOBID();
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
		m_RenderInfo.skin.aniBoneIndex = comAnimator->GetBoneMatStoredIndex();
	}
}

void DOFont::DoFuncFromMouse(DirectX::Mouse::ButtonStateTracker::ButtonState state, DirectX::MOUSEBUTTONINDEX index)
{
	if (m_Funcs)
	{
		auto& indices = m_Funcs->buttonIndices;
		auto& states = m_Funcs->states;

		for (size_t i = 0; i < m_Funcs->funcs.size(); i++)
		{
			if (indices[i] == index && states[i] == state)
			{
				m_Funcs->funcs[i]();
			}
		}
	}
}

void DOFont::AddPixelFunc(std::function<void()> func, 
	DirectX::Mouse::ButtonStateTracker::ButtonState state, DirectX::MOUSEBUTTONINDEX index)
{
	if (m_Funcs == nullptr)
	{
		m_Funcs = std::make_unique<RenderFuncFromMouse>();
	}

	m_Funcs->funcs.push_back(func);
	m_Funcs->states.push_back(state);
	m_Funcs->buttonIndices.push_back(index);
}

void DOFont::Update(float delta)
{
	if (m_Text.size())
	{
		RenderFont addedFont(m_FontName, m_Text);
		addedFont.pos = m_Pos;
		addedFont.fontHeight = m_FontHeight;
		addedFont.color = m_Color;
		addedFont.drawSize = &m_DrawSize;
		addedFont.benchmark = m_Benchmark;
		addedFont.pixelColID = FONTRENDERERID(GetDeviceOBID());

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