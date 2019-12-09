#include "AnimationStructs.h"

using namespace DirectX;

float Ani::SkinnedData::GetClipStartTime(const std::string& clipName) const
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

float Ani::SkinnedData::GetClipEndTime(const std::string& clipName) const
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

void Ani::SkinnedData::GetFinalTransforms(const std::string& clipName, float timePos, std::vector<CGH::MAT16>& finalTransforms)
{
	std::vector<CGH::MAT16> combinedMats;
	finalTransforms.clear();
	finalTransforms.resize(m_LocalTrnasform.size());
	combinedMats.resize(m_LocalTrnasform.size());

	CalLocalTransformFromAnimation(clipName, timePos);

	for (size_t i = 0; i < finalTransforms.size(); i++)
	{
		XMMATRIX offsetMatrix = XMLoadFloat4x4(m_BoneOffsets[i]);
		XMMATRIX currBoneMat = XMLoadFloat4x4(m_LocalTrnasform[i]);
		XMMATRIX parentMat = XMMatrixIdentity();
		XMMATRIX combined = XMMatrixIdentity();

		if (m_BoneHierarchy[i] != -1)
		{
			parentMat = XMLoadFloat4x4(combinedMats[m_BoneHierarchy[i]]);
		}

		combined = parentMat * currBoneMat;
		XMStoreFloat4x4(combinedMats[i], combined);
		XMStoreFloat4x4(finalTransforms[i], offsetMatrix * combined);
	}
}

void Ani::SkinnedData::CalLocalTransformFromAnimation(const std::string& clipName, float timePos)
{
	auto aniIter = m_Animations.find(clipName);
	assert(aniIter != m_Animations.end() && ("This skinned don't have [" + clipName + "] animation").c_str());

	for (auto& it : aniIter->second.animBones)
	{
		XMMATRIX mat = XMMatrixIdentity();
		XMVECTOR zero = XMVectorSet(0, 0, 0, 1);

		if (it.posKeys.size())
		{
			XMVECTOR s = GetAnimationKeyOnTick(it.scaleKeys, timePos);
			XMVECTOR p = GetAnimationKeyOnTick(it.posKeys, timePos);
			XMVECTOR r = GetAnimationKeyOnTick(it.rotKeys, timePos);

			mat = XMMatrixAffineTransformation(s, zero, r, p);
			XMStoreFloat4x4(m_LocalTrnasform[it.localMatIndex], mat);
		}
		else if (it.trafoKeys.size())
		{

		}
		else
		{
			assert(false);
		}
	}
}

DirectX::XMVECTOR XM_CALLCONV Ani::SkinnedData::GetAnimationKeyOnTick(const std::vector<TimeValue<DirectX::XMFLOAT3>>& values, float timePos) const
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
					(values[i + 1].time - values[i].time);

				DirectX::XMVECTOR prev = DirectX::XMLoadFloat3(&values[i].value);
				DirectX::XMVECTOR next = DirectX::XMLoadFloat3(&values[i + 1].value);

				result = DirectX::XMVectorLerp(prev, next, lerpPercent);

				break;
			}
		}
	}

	return result;
}

DirectX::XMVECTOR XM_CALLCONV Ani::SkinnedData::GetAnimationKeyOnTick(const std::vector<TimeValue<DirectX::XMFLOAT4>>& values, float timePos) const
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
					(values[i + 1].time - values[i].time);

				DirectX::XMVECTOR prev = DirectX::XMLoadFloat4(&values[i].value);
				DirectX::XMVECTOR next = DirectX::XMLoadFloat4(&values[i + 1].value);

				result = DirectX::XMQuaternionSlerp(prev, next, lerpPercent);

				break;
			}
		}
	}

	return result;
}
