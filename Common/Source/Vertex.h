#pragma once
#include "BaseClass.h"

struct Vertex
{
	Vertex() = default;
	Vertex(physx::PxVec3 _pos, physx::PxVec3 _normal, physx::PxVec2 _uv)
	{
		position = _pos;
		normal = normal;
		uv = _uv;
	}

	physx::PxVec3 position = { 0,0,0 };
	physx::PxVec3 normal = { 0,0,-1 };
	physx::PxVec2 uv = { 0,0 };
};


struct SkinnedVertex
{
	physx::PxVec3	position;
	physx::PxVec3	normal;
	physx::PxVec2	uv;
	physx::PxVec3	boneWeights;
	unsigned char	boneIndices[4];
};


struct PointBaseVertex
{
	unsigned int		type = 0;
	unsigned int		cbIndex = 0;
	physx::PxVec3		size = { 0,0,0 };
	physx::PxVec4		color = { 0,0,0,0 };
};
