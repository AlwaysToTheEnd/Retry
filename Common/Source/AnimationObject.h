#pragma once
#include "GameObject.h"
#include "BaseClass.h"
#include <string>
#include <unordered_map>
#include "AnimationStructs.h"


class AnimationObject
{
public:
	AnimationObject()
		: m_RootNode(nullptr)
		, m_AnimTicksPerSecond(0)
	{
		m_CurrAnimation = m_Anims.end();
	}

	virtual ~AnimationObject()
	{
		if (m_RootNode)
		{
			delete m_RootNode;
		}
	}

	void Init();
	void Update();

public:
	bool SetAnimation(std::string name);
	void SetAnimationTime(unsigned int tickTime) { m_AnimTicksPerSecond = tickTime; }

	void GetAnimationNames(std::vector<std::string>& out);
	std::string GetCurrAniName();

private:
	void SetupBoneMatrixPtrs(Ani::Node* node);
	void Update(Ani::Node* node);
	void UpdateSkinnedMesh(Ani::Node* node);
	void CalFinalTransform();

	DirectX::XMVECTOR XM_CALLCONV GetAnimationKeyOnTick(const std::vector<Ani::TimeValue<DirectX::XMFLOAT3>>& values);
	DirectX::XMVECTOR XM_CALLCONV GetAnimationKeyOnTick(const std::vector<Ani::TimeValue<DirectX::XMFLOAT4>>& values);

public:

	std::vector<std::unique_ptr<Ani::Mesh>>			m_GlobalMeshes;
	std::vector<std::unique_ptr<Ani::AniMaterial>>	m_GlobalMaterials;

	/////////
	std::unordered_map<std::string, std::unique_ptr<Ani::Animation>>			m_Anims;
	std::unordered_map<std::string, std::unique_ptr<Ani::Animation>>::iterator	m_CurrAnimation;

	std::unordered_map<std::string, DirectX::XMFLOAT4X4*>						m_TransformationMatPtrs;
	unsigned int																m_AnimTicksPerSecond;
};