#pragma once
#include "GraphicBase.h"
#define MAXTEXTURENUM 100
#define MAXLIGHT 15

enum DX12_SHADER_TYPE
{
	DX12_SHADER_VERTEX,
	DX12_SHADER_PIXEL,
	DX12_SHADER_GEOMETRY,
	DX12_SHADER_HULL,
	DX12_SHADER_DOMAIN,
	DX12_SHADER_COMPUTE,
	DX12_SHADER_TYPE_COUNT
};

struct DX12_LIGHT_VALUE
{
	physx::PxVec3	lightColor = { 0.5f, 0.5f, 0.5f };
	float			falloffStart = 1.0f;				// point/spot light only
	physx::PxVec3	direction = { 0.5f, 0.5f, 0.5f };	// directional/spot light only
	float			falloffEnd = 10.0f;					// point/spot light only
	physx::PxVec3	position = { 0.00f, 0.0f, 0.0f };	// point/spot light only
	float			spotPower = 50.0f;					// spot light only
	unsigned int	type;
	unsigned int	pad0;
	unsigned int	pad1;
	unsigned int	pad2;
};

struct DX12ObjectConstants
{
	physx::PxMat44	world;
	physx::PxVec3	scale = { 1,1,1 };
	int				diffuseMapIndex = -1;
	int				normalMapIndex = -1;
	int				materialIndex = -1;
	int				PrevAniBone;
	float			blendFactor;
};

struct DX12OnlyTexObjectConstants
{
	physx::PxMat44	world;
	int				textureIndex = -1;
	int				pad0;
	int				pad1;
	int				pad2;
};

struct DX12UIInfomation
{
	physx::PxVec4	color;
	physx::PxVec3	pos;
	physx::PxVec2	size;
	int				uiType = 0;
	int				textureIndex = -1;
};

struct DX12PassConstants
{
	physx::PxMat44		view;
	physx::PxMat44		invView;
	physx::PxMat44		proj;
	physx::PxMat44		invProj;
	physx::PxMat44		viewProj;
	physx::PxMat44		invViewProj;
	physx::PxMat44		rightViewProj;
	physx::PxMat44		orthoMatrix;
	physx::PxVec2		renderTargetSize = { 0.0f, 0.0f };
	physx::PxVec2		invRenderTargetSize = { 0.0f, 0.0f };
	physx::PxVec4		ambientLight = { 0.0f, 0.0f, 0.0f, 1.0f };
	physx::PxVec3		eyePosW = { 0.0f, 0.0f, 0.0f };
	unsigned int		samplerIndex = 0;
	DX12_LIGHT_VALUE	lights[MAXLIGHT];
};
