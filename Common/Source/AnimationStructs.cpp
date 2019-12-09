#include "AnimationStructs.h"

using namespace DirectX;

float Ani::SkinnedData::GetClipStartTime(const std::string& clipName) const
{
	return 0.0f;
}

float Ani::SkinnedData::GetClipEndTime(const std::string& clipName) const
{
	return 0.0f;
}

void Ani::SkinnedData::GetFinalTransforms(const std::string& clipName, float timePos, std::vector<CGH::MAT16>& finalTransforms)
{
	finalTransforms.clear();
	CalLocalTransformFromAnimation(clipName, timePos);


}

void Ani::SkinnedData::CalLocalTransformFromAnimation(const std::string& clipName, float timePos)
{
	auto aniIter = m_Animations.find(clipName);
	assert(aniIter != m_Animations.end() && ("This skinned don't have [" + clipName + "] animation").c_str());

	for (auto& it : aniIter->second.animBones)
	{
		XMMATRIX mat = XMMatrixIdentity();

		if (it.posKeys.size())
		{
			XMVECTOR s = GetAnimationKeyOnTick(it.scaleKeys, timePos);
			XMVECTOR p = GetAnimationKeyOnTick(it.posKeys, timePos);
			XMVECTOR r = GetAnimationKeyOnTick(it.rotKeys, timePos);
			XMVECTOR zero = XMVectorSet(0, 0, 0, 1);

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

DirectX::XMVECTOR XM_CALLCONV Ani::SkinnedData::GetAnimationKeyOnTick(const std::vector<TimeValue<DirectX::XMFLOAT3>>& values, float timePos)
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

DirectX::XMVECTOR XM_CALLCONV Ani::SkinnedData::GetAnimationKeyOnTick(const std::vector<TimeValue<DirectX::XMFLOAT4>>& values, float timePos)
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
