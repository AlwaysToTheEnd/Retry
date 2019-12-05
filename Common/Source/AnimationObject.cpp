#include "AnimationObject.h"
#include "d3dApp.h"

using namespace DirectX;
using namespace std;

void AnimationObject::Init()
{
	//AddComponent(COMPONENTTYPE::COM_GRAPHIC);

	SetupBoneMatrixPtrs(m_RootNode);
}

void AnimationObject::Update()
{
	//TODO: Get delta tick
	UINT deltaTick = 1;
	
	if (m_CurrAnimation != m_Anims.end())
	{
		m_AnimTicksPerSecond = 4000;

		CalFinalTransform();
		Update(m_RootNode);
		UpdateSkinnedMesh(m_RootNode);
	}
}

bool AnimationObject::SetAnimation(string name)
{
	m_CurrAnimation = m_Anims.find(name);

	if (m_CurrAnimation == m_Anims.end())
	{
		return false;
	}

	m_AnimTicksPerSecond = 0;
	return true;
}

void AnimationObject::GetAnimationNames(vector<string>& out)
{
	out.clear();

	const size_t numAnims = m_Anims.size();
	out.resize(numAnims);

	UINT currIndex = 0;
	for (auto& it : m_Anims)
	{
		out[currIndex++] = it.first;
	}
}

std::string AnimationObject::GetCurrAniName()
{
	string result;

	if (m_CurrAnimation != m_Anims.end())
	{
		result = m_CurrAnimation->first;
	}

	return result;
}

void AnimationObject::SetupBoneMatrixPtrs(Ani::Node* node)
{
	assert(node);

	if (node->m_Meshes.size())
	{
		for (auto& it : node->m_Meshes)
		{
			it->m_CurrentBoneMatrices.resize(it->m_Bones.size());
			for (auto& it2 : it->m_Bones)
			{
				Ani::Node* currNode = m_RootNode->SearchNodeByName(it2.m_Name);
				it->m_BoneMatrixPtrs.push_back(&currNode->m_CombinedTransformationMatrix);
			}
		}
	}

	m_TransformationMatPtrs.insert({ node->m_Name, node->m_TransformMat });

	for (auto& it : node->m_Children)
	{
		SetupBoneMatrixPtrs(it);
	}
}

void AnimationObject::CalFinalTransform()
{
	for (auto& it : m_CurrAnimation->second->m_AnimBones)
	{
		auto finalMatPtrIter = m_TransformationMatPtrs.find(it.m_BoneName);

		if (finalMatPtrIter != m_TransformationMatPtrs.end())
		{
			bool isCurrAniBoneDataStoredMatrixData = it.IsMatrixDataType();
			XMVECTOR S = GetAnimationKeyOnTick(it.m_ScaleKeys);
			XMVECTOR P = GetAnimationKeyOnTick(it.m_PosKeys);
			XMVECTOR Q = GetAnimationKeyOnTick(it.m_RotKeys);
			XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
			XMMATRIX mat = XMMatrixAffineTransformation(S, zero, Q, P);
			XMStoreFloat4x4(finalMatPtrIter->second, mat);
		}
		else
		{
			assert(false);
		}
	}
}

void AnimationObject::Update(Ani::Node* node)
{
	assert(node);
	
	if (node->m_Parent)
	{
		XMMATRIX currNodeTransformMat = XMLoadFloat4x4(node->m_TransformMat);
		XMMATRIX parentNodeTranMat = XMLoadFloat4x4(node->m_Parent->m_CombinedTransformationMatrix);
		XMMATRIX result = XMMatrixMultiply(currNodeTransformMat, parentNodeTranMat);
		XMStoreFloat4x4(node->m_CombinedTransformationMatrix, result);
	}

	for (auto& it : node->m_Children)
	{
		Update(it);
	}
}

void AnimationObject::UpdateSkinnedMesh(Ani::Node* node)
{
	assert(node);

	if (node->m_Meshes.size())
	{
		for (auto& it : node->m_Meshes)
		{
			for (size_t i = 0; i < it->m_Bones.size(); i++)
			{
				XMMATRIX offsetMat = XMLoadFloat4x4(it->m_Bones[i].m_OffsetMatrix);
				XMMATRIX boneMat = XMLoadFloat4x4(*it->m_BoneMatrixPtrs[i]);
				XMMATRIX result = XMMatrixMultiply(offsetMat, boneMat);
				XMStoreFloat4x4(it->m_CurrentBoneMatrices[i], result);
			}
			int num = 10;
		}
	}

	for (auto& it : node->m_Children)
	{
		UpdateSkinnedMesh(it);
	}
}