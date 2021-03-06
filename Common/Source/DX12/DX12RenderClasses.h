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

struct DX12ObjectConstants
{
	physx::PxMat44	world;
	physx::PxVec3	scale = { 1,1,1 };
	int				diffuseMapIndex = -1;
	int				normalMapIndex = -1;
	int				materialIndex = -1;
	int				PrevAniBone = -1;
	float			blendFactor = 0.0f;
	float			boundSphereRad = 1.0f;
	int				objectID = -1;
	int				pad1 = 0;
	int				pad2 = 0;
};

struct DX12OnlyTexObjectConstants
{
	physx::PxMat44	world;
	int				textureIndex = -1;
	int				ObjectID = -1;
	int				pad1 = 0;
	int				pad2 = 0;
};

struct DX12UIInfomation
{
	physx::PxVec4	color;
	physx::PxVec3	pos;
	physx::PxVec2	size;
	int				uiType = 0;
	int				textureIndex = -1;
	int				objectID = -1;
};

struct DX12LightInfomation
{
	physx::PxVec4	posnAngle = { 0.0f, 0.0f, 0.0f, 1.0f };	
	physx::PxVec4	lightColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	physx::PxVec4	falloffAndPower = { 1.0f, 10.0f, 50.0f, 1.0f };
	physx::PxVec4	dir = { 0, 0, 1, 0 };
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
	unsigned int		renderTargetSizeX = 0;
	unsigned int		renderTargetSizeY = 0;
	physx::PxVec2		invRenderTargetSize = { 0.0f, 0.0f };
	physx::PxVec4		ambientLight = { 0.0f, 0.0f, 0.0f, 1.0f };
	physx::PxVec3		eyePosW = { 0.0f, 0.0f, 0.0f };
	unsigned int		samplerIndex = 0;
	physx::PxVec2		mousePos;
	physx::PxVec2		pad;
};
