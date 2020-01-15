#pragma once
#include <foundation/PxVec2.h>
#include <foundation/PxVec3.h>
#include <foundation/PxVec4.h>
#include <foundation/PxMat44.h>

struct ObjectConstants
{
	physx::PxMat44	world;
	physx::PxVec3	scale = { 1,1,1 };
	int				diffuseMapIndex = -1;
	int				normalMapIndex = -1;
	int				materialIndex = -1;
	int				aniBoneIndex = -1;
	unsigned int	primitive;
	int				dynamicBufferIndex;
	int				pad0;
	int				pad1;
	int				pad2;
};

struct OnlyTexObjectConstants
{
	physx::PxMat44	world;
	int				textureIndex = -1;
	int				pad0;
	int				pad1;
	int				pad2;
};

struct UIInfomation
{
	physx::PxVec4	color;
	physx::PxVec3	pos;
	physx::PxVec2	size;
	int				uiType = 0;
	int				textureIndex = -1;
};

struct PassConstants
{
	physx::PxMat44	view;
	physx::PxMat44	invView;
	physx::PxMat44	proj;
	physx::PxMat44	invProj;
	physx::PxMat44	viewProj;
	physx::PxMat44	invViewProj;
	physx::PxMat44	rightViewProj;
	physx::PxMat44	orthoMatrix;
	physx::PxVec3	eyePosW = { 0.0f, 0.0f, 0.0f };
	physx::PxVec2	renderTargetSize = { 0.0f, 0.0f };
	physx::PxVec2	invRenderTargetSize = { 0.0f, 0.0f };
	physx::PxVec4	ambientLight = { 0.0f, 0.0f, 0.0f, 1.0f };
	unsigned int	samplerIndex = 0;

	//Light Lights[16];
};