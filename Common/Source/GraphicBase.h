#pragma once
#include <foundation/PxMat44.h>
#include <foundation/PxVec2.h>
#include <foundation/PxVec3.h>
#include <string>
#include <vector>
#include <unordered_map>
#define BONEMAXMATRIX 160

namespace CGH
{
	enum MESH_TYPE
	{
		MESH_NORMAL,
		MESH_SKINED,
		MESH_NONE
	};
}

enum RENDER_TYPE
{
	RENDER_NONE,
	RENDER_MESH,
	RENDER_BOX,
	RENDER_PLANE,
	RENDER_TEX_PLANE,
	RENDER_UI,
};

struct AniBoneMat
{
	physx::PxMat44		bones[BONEMAXMATRIX];
};

struct RenderMeshInfo
{
	int	aniBoneIndex = -1;
};

struct RenderPointInfo
{
	physx::PxVec3 size;
	physx::PxVec4 color;
};

struct RenderTexturePointInfo
{
	physx::PxVec3 size;
};

struct RenderInfo
{
	RenderInfo(RENDER_TYPE _type)
		:type(_type)
	{

	}

	RenderInfo(const RenderInfo& src)
	{
		std::memcpy(&point, &src.point, sizeof(RenderPointInfo));
		type = src.type;
		world = src.world;
		meshOrTextureName = src.meshOrTextureName;
	}

	~RenderInfo()
	{

	}

	RenderInfo& operator=(const RenderInfo& src)
	{
		std::memcpy(&point, &src.point, sizeof(RenderPointInfo));
		type = src.type;
		world = src.world;
		meshOrTextureName = src.meshOrTextureName;

		return *this;
	}

	RENDER_TYPE		type;
	physx::PxMat44	world;
	std::string		meshOrTextureName;

	union
	{
		RenderMeshInfo mesh;
		RenderPointInfo point;
		RenderTexturePointInfo texPoint;
	};
};

struct RenderFont
{
	enum class FONTBENCHMARK
	{
		LEFT,
		CENTER,
		RIGHT
	};

	RenderFont(const std::wstring& fontName, const std::wstring& text)
		: printString(text)
		, fontIndex(GetFontIndex(fontName))
		, benchmark(FONTBENCHMARK::LEFT)
	{

	}

	static unsigned int GetFontIndex(const std::wstring& fontName);
	static std::vector<std::wstring> fontNames;

	const unsigned int	fontIndex;
	std::wstring		printString;
	float				rotation = 0;
	int					fontHeight = -1;
	FONTBENCHMARK		benchmark;
	physx::PxVec3		pos = { 0, 0, 0 };
	physx::PxVec4		color = { 0,0,0,1 };
	physx::PxVec2*		drawSize = nullptr; //Output: If not nullptr, Get font drawSize from Device 
};


struct Material
{
	physx::PxVec4	diffuseAlbedo = { 1,1,1,1 };
	physx::PxVec3	fresnel0 = { 0.01f,0.01f,0.01f };
	float			roughness = 0.25f;

	int				diffuseMapIndex = -1;
	int				normalMapIndex = -1;
	int				materialPad1;
	int				materialPad2;
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

struct Light
{
	physx::PxVec3 strength = { 0.5f,0.5f,0.5f };
	float falloffStart = 1.0f;
	physx::PxVec3 direction = { 0,-1.0f,0 };
	float falloffEnd = 10.0f;
	physx::PxVec3 position = { 0,0,0 };
	float spotPower = 64.0f;
};
