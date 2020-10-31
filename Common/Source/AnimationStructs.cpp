#include "AnimationStructs.h"
#include "GraphicBase.h"
#include <d3d12.h>
#include <SimpleMath.h>

using namespace DirectX;

unsigned int Ani::SkinnedData::GetClipStartTime(const std::string& clipName) const
{
	auto aniIter = m_Animations.find(clipName);
	assert(aniIter != m_Animations.end() && ("This skinned don't have [" + clipName + "] animation").c_str());

	auto boneIter = aniIter->second.animBones.begin();

	if (boneIter == aniIter->second.animBones.end())
	{
		return -1.0f;
	}

	if (boneIter->posKeys.size())
	{
		return boneIter->posKeys.front().time;
	}
	else if (boneIter->trafoKeys.size())
	{
		return boneIter->trafoKeys.front().time;
	}
	else
	{
		assert(false);
	}

	return -1.0f;
}

unsigned int Ani::SkinnedData::GetClipEndTime(const std::string& clipName) const
{
	auto aniIter = m_Animations.find(clipName);
	assert(aniIter != m_Animations.end() && ("This skinned don't have [" + clipName + "] animation").c_str());

	auto boneIter = aniIter->second.animBones.begin();

	if (boneIter == aniIter->second.animBones.end())
	{
		return -1.0f;
	}

	if (boneIter->posKeys.size())
	{
		return boneIter->posKeys.back().time;
	}
	else if (boneIter->trafoKeys.size())
	{
		return boneIter->trafoKeys.back().time;
	}
	else
	{
		assert(false);
	}

	return -1.0f;
}

void Ani::SkinnedData::GetFinalTransforms(
	const std::string& clipName, 
	unsigned long long timePos,
	AniBoneMat& finalTransforms) const
{
	std::vector<physx::PxMat44> localTransform;
	std::vector<physx::PxMat44> combinedMats;

	localTransform.resize(m_FrameHierarchy.size(), physx::PxMat44(physx::PxIDENTITY::PxIdentity));
	combinedMats.resize(m_FrameHierarchy.size());

	assert(BONEMAXMATRIX >= m_BoneOffsets.size());

	CalLocalTransformFromAnimation(clipName, localTransform, timePos);

	for (size_t i = 0; i < localTransform.size(); i++)
	{
		int targetFrameIndex = m_FrameHierarchy[i];
		XMMATRIX currFrameMat = XMLoadFloat4x4(localTransform[i]);
		XMMATRIX parentMat = XMMatrixIdentity();
		XMMATRIX combined = XMMatrixIdentity();

		if (targetFrameIndex != -1)
		{
			assert(targetFrameIndex < i);
			parentMat = XMLoadFloat4x4(combinedMats[targetFrameIndex]);

			combined = currFrameMat * parentMat;
			XMStoreFloat4x4(combinedMats[i], combined);
		}
		else
		{
			XMStoreFloat4x4(combinedMats[i], currFrameMat);
		}
	}

	for (size_t i = 0; i < m_BoneOffsets.size(); i++)
	{
		unsigned int index = m_BoneOffsetsFrameIndex[i];
		XMMATRIX boneOffset = XMLoadFloat4x4(m_BoneOffsets[i]);
		XMMATRIX combined = XMLoadFloat4x4(combinedMats[index]);

		XMStoreFloat4x4(finalTransforms.bones[i], XMMatrixTranspose(XMMatrixMultiply(boneOffset ,combined)));
	}
}

void Ani::SkinnedData::GetAnimationNames(std::vector<std::string>& out) const
{
	out.clear();
	for (auto& it : m_Animations)
	{
		out.push_back(it.first);
	}
}

bool Ani::SkinnedData::CheckAnimation(const std::string& key) const
{
	return m_Animations.find(key) != m_Animations.end();
}

void Ani::SkinnedData::CalLocalTransformFromAnimation(const std::string& clipName, std::vector<physx::PxMat44>& LocalTransforms, unsigned long long timePos) const
{
	auto aniIter = m_Animations.find(clipName);
	assert(aniIter != m_Animations.end() && ("This skinned don't have [" + clipName + "] animation").c_str());

	for (auto& it : aniIter->second.animBones)
	{
		XMMATRIX mat = XMMatrixIdentity();
		XMVECTOR zero = XMVectorSet(0, 0, 0, 1);

		if (it.posKeys.size())
		{
			XMVECTOR s =  GetAnimationKeyOnTick(reinterpret_cast<const std::vector<TimeValue<DirectX::XMFLOAT3>>&>(it.scaleKeys), timePos);
			XMVECTOR t = GetAnimationKeyOnTick(reinterpret_cast<const std::vector<TimeValue<DirectX::XMFLOAT3>>&>(it.posKeys), timePos);
			XMVECTOR r = GetAnimationKeyOnTick(reinterpret_cast<const std::vector<TimeValue<DirectX::XMFLOAT4>>&>(it.rotKeys), timePos);

			mat = XMMatrixAffineTransformation(s, zero, r, t);
			XMStoreFloat4x4(LocalTransforms[it.localMatIndex], mat);
		}
		else if (it.trafoKeys.size())
		{
			XMStoreFloat4x4(LocalTransforms[it.localMatIndex], GetAnimationKeyOnTick(it.trafoKeys, timePos));
		}
		else
		{
			assert(false);
		}
	}
}

DirectX::XMVECTOR XM_CALLCONV Ani::SkinnedData::GetAnimationKeyOnTick(const std::vector<TimeValue<DirectX::XMFLOAT3>>& values, unsigned long long timePos) const
{

	DirectX::XMVECTOR result = DirectX::XMVectorSet(0, 0, 0, 1);

	if (timePos <= values.front().time)
	{
		result = DirectX::XMLoadFloat3(&values.front().value);
	}
	else if (timePos >= values.back().time)
	{
		result = DirectX::XMLoadFloat3(&values.back().value);
	}
	else
	{
		for (size_t i = 0; i < values.size() - 1; i++)
		{
			if (timePos >= values[i].time && timePos <= values[i + 1].time)
			{
				float lerpPercent =
					(timePos - values[i].time) /
					static_cast<float>((values[i + 1].time - values[i].time));

				DirectX::XMVECTOR prev = DirectX::XMLoadFloat3(&values[i].value);
				DirectX::XMVECTOR next = DirectX::XMLoadFloat3(&values[i + 1].value);

				result = DirectX::XMVectorLerp(prev, next, lerpPercent);

				break;
			}
		}
	}

	return result;
}

DirectX::XMVECTOR XM_CALLCONV Ani::SkinnedData::GetAnimationKeyOnTick(const std::vector<TimeValue<DirectX::XMFLOAT4>>& values, unsigned long long timePos) const
{
	DirectX::XMVECTOR result = DirectX::XMVectorSet(0, 0, 0, 1);

	if (timePos <= values.front().time)
	{
		result = DirectX::XMLoadFloat4(&values.front().value);
	}
	else if (timePos >= values.back().time)
	{
		result = DirectX::XMLoadFloat4(&values.back().value);
	}
	else
	{
		for (size_t i = 0; i < values.size() - 1; i++)
		{
			if (timePos >= values[i].time && timePos <= values[i + 1].time)
			{
				float lerpPercent =
					(timePos - values[i].time) /
					static_cast<float>((values[i + 1].time - values[i].time));

				DirectX::XMVECTOR prev = DirectX::XMLoadFloat4(&values[i].value);
				DirectX::XMVECTOR next = DirectX::XMLoadFloat4(&values[i + 1].value);

				result = DirectX::XMQuaternionSlerp(prev, next, lerpPercent);
				break;
			}
		}
	}

	return result;
}

DirectX::XMMATRIX XM_CALLCONV Ani::SkinnedData::GetAnimationKeyOnTick(const std::vector<TimeValue<physx::PxMat44>>& values, unsigned long long timePos) const
{
	DirectX::XMMATRIX result = DirectX::XMMatrixIdentity();

	if (timePos <= values.front().time)
	{
		result = DirectX::XMLoadFloat4x4(values.front().value);
	}
	else if (timePos >= values.back().time)
	{
		result = DirectX::XMLoadFloat4x4(values.back().value);
	}
	else
	{
		for (size_t i = 0; i < values.size() - 1; i++)
		{
			if (timePos >= values[i].time && timePos <= values[i + 1].time)
			{
				float lerpPercent =
					(timePos - values[i].time) /
					static_cast<float>((values[i + 1].time - values[i].time));
				DirectX::XMMATRIX prev = DirectX::XMLoadFloat4x4(values[i].value);
				DirectX::XMMATRIX next = DirectX::XMLoadFloat4x4(values[i + 1].value);
				
				result = DirectX::SimpleMath::Matrix::Lerp(prev, next, lerpPercent);
				break;
			}
		}
	}

	return result;
}
