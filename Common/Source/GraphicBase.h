#pragma once
#include <foundation/PxMat44.h>
#include <foundation/PxVec2.h>
#include <foundation/PxVec3.h>
#include <string>
#include <vector>
#include <unordered_map>
#include "Vertex.h"
#define BONEMAXMATRIX 160

namespace CGH
{
	enum MESH_TYPE
	{
		NORMAL_MESH,
		SKINNED_MESH,
		HEIGHTFIELD_MESH,
		MESH_TYPE_COUNT,
	};
};

enum RENDER_TYPE
{
	RENDER_NONE,
	RENDER_MESH,
	RENDER_SKIN,
	RENDER_HEIGHT_FIELD,
	RENDER_BOX,
	RENDER_PLANE,
	RENDER_2DPLANE,
	RENDER_UI,
};

struct RenderSkinnedMeshInfo
{
	int	aniBoneIndex = -1;
};

struct AniBoneMat
{
	physx::PxMat44		bones[BONEMAXMATRIX];
};

struct RenderPointInfo
{
	physx::PxVec3 size;
	physx::PxVec4 color;
};

struct RenderUIInfo
{
	int				uiType;
	physx::PxVec2	size;
	physx::PxVec4	color;
};

struct RenderInfo
{
	RenderInfo(RENDER_TYPE _type)
		: type(_type)
		, world(physx::PxIdentity)
		, scale(1,1,1)
	{

	}

	RenderInfo(const RenderInfo& src)
	{
		std::memcpy(&uiInfo, &src.uiInfo, sizeof(RenderUIInfo));
		type = src.type;
		world = src.world;
		scale = src.scale;
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
		scale = src.scale;
		meshOrTextureName = src.meshOrTextureName;

		return *this;
	}

	RENDER_TYPE		type;
	physx::PxMat44	world;
	physx::PxVec3	scale;
	std::string		meshOrTextureName;

	union
	{
		RenderSkinnedMeshInfo	skin;
		RenderPointInfo			point;
		RenderUIInfo			uiInfo;
	};
};

struct RenderFont
{
	RenderFont(const std::wstring& fontName, const std::wstring& text)
		: printString(text)
		, fontIndex(GetFontIndex(fontName))
		, benchmark(0,0)
	{

	}

	static unsigned int GetFontIndex(const std::wstring& fontName);
	static std::vector<std::wstring> fontNames;

	const unsigned int	fontIndex;
	std::wstring		printString;
	float				rotation = 0;
	int					fontHeight = -1;
	physx::PxVec2		benchmark;
	physx::PxVec3		pos = { 0, 0, 0 };
	physx::PxVec4		color = { 0,0,0,1 };
	physx::PxVec2*		drawSize = nullptr; //Output: If not nullptr, Get font drawSize from Device 
};


struct Material
{
	physx::PxVec4	diffuseAlbedo = { 1,1,1,1 };
	physx::PxVec3	fresnel0 = { 0.01f,0.01f,0.01f };
	float			roughness = 0.25f;
};

struct SubmeshData
{
	std::string		material;
	std::string		diffuseMap;
	std::string		normalMap;
	unsigned int	numVertex = 0;
	unsigned int	vertexOffset = 0;
	unsigned int	numIndex = 0;
	unsigned int	indexOffset = 0;
};

struct MeshObject
{
	size_t	GetTotalVertexNum() const;
	size_t	GetStartVertexOffset() const;
	size_t	GetTotalIndexNum() const;
	size_t	GetStartIndexOffset() const;
	bool	IsOneSub() const { return subs.size() == 1; }

	unsigned int	primitiveType = 0;
	std::unordered_map<std::string, SubmeshData> subs;
};

enum class DYNAMIC_BUFFER_EDIT_MOD
{
	CIRCLE_AREA_UP,
	CIRCLE_AREA_DOWN,
	CON_SHAPE_UP,
	CON_SHAPE_DOWN,
	CIRCLE_AREA_FLATNESS,
};