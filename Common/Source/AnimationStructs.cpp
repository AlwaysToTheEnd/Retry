#include "AnimationStructs.h"


float Ani::SkinnedData::GetClipStartTime(const std::string& clipName) const
{
	return 0.0f;
}

float Ani::SkinnedData::GetClipEndTime(const std::string& clipName) const
{
	return 0.0f;
}

void Ani::SkinnedData::GetFinalTransforms(const std::string& clipName, float timePos, std::vector<CGH::MAT16>& finalTransforms) const
{
}
