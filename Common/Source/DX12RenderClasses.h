#pragma once

#include <DirectXMath.h>
#include <string>
#include <unordered_map>
#include "BaseClass.h"
#define BONEMAXMATRIX 160

struct ObjectConstants
{
	physx::PxMat44		world;
	unsigned int	materialIndex = 0;
	int				aniBoneIndex = -1;
	unsigned int	meshType;
	unsigned int	primitive;
};

struct RenderInfo
{
	physx::PxMat44	world;
	std::string	meshName;
	int			aniBoneIndex = -1;
};

struct Material
{
	DirectX::XMFLOAT4	diffuseAlbedo = { 1,1,1,1 };
	DirectX::XMFLOAT3	fresnel0 = { 0.01f,0.01f,0.01f };
	float		roughness = 0.25f;

	int			diffuseMapIndex = -1;
	int			normalMapIndex = -1;
	int			materialPad1;
	int			materialPad2;
};

struct Light
{
	DirectX::XMFLOAT3 strength = { 0.5f,0.5f,0.5f };
	float falloffStart = 1.0f;
	DirectX::XMFLOAT3 direction = { 0,-1.0f,0 };
	float falloffEnd = 10.0f;
	DirectX::XMFLOAT3 position = { 0,0,0 };
	float spotPower = 64.0f;
};

struct PassConstants
{
	physx::PxMat44 view;
	physx::PxMat44 invView;
	physx::PxMat44 proj;
	physx::PxMat44 invProj;
	physx::PxMat44 viewProj;
	physx::PxMat44 invViewProj;
	physx::PxMat44 rightViewProj;
	physx::PxMat44 shadowMapMatrix;
	DirectX::XMFLOAT3 eyePosW = { 0.0f, 0.0f, 0.0f };
	float cbPerObjectPad1 = 0.0f;
	DirectX::XMFLOAT2 renderTargetSize = { 0.0f, 0.0f };
	DirectX::XMFLOAT2 invRenderTargetSize = { 0.0f, 0.0f };

	DirectX::XMFLOAT4 ambientLight = { 0.0f, 0.0f, 0.0f, 1.0f };

	//Light Lights[16];
};

struct SubmeshData
{
	std::string		material;
	unsigned int	numVertex = 0;
	unsigned int	vertexOffset = 0;
	unsigned int	numIndex = 0;
	unsigned int	indexOffset = 0;
};

struct MeshObject
{
	unsigned int	primitiveType;
	CGH::MESH_TYPE	type = CGH::MESH_TYPE::MESH_NORMAL;

	std::unordered_map<std::string, SubmeshData> subs;
};

struct AniBoneMat
{
	physx::PxMat44		bones[BONEMAXMATRIX];
};

struct RenderFont
{
	RenderFont(const std::wstring& fontName, const std::wstring& text)
		: printString(text)
		, fontIndex(GetFontIndex(fontName))
	{

	}

	static unsigned int GetFontIndex(const std::wstring& fontName);
	static std::vector<std::wstring> fontNames;

	const unsigned int fontIndex;
	const std::wstring& printString;
	float rotation = 0;
	DirectX::XMFLOAT3 pos = { 0, 0, 0 };
	DirectX::XMFLOAT3 scale = { 1, 1, 1 };
	DirectX::XMFLOAT4 color = { 0,0,0,1 };
};