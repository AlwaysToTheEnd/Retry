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
	RENDER_LIGHT,
	RENDER_UI,
};

enum LIGHT_TYPE
{
	LIGHT_TYPE_DIRECTIONAL,
	LIGHT_TYPE_POINT,
	LIGHT_TYPE_SPOT,
	LIGHT_TYPE_COUNT,
};

struct AniBoneMat
{
	physx::PxMat44		bones[BONEMAXMATRIX];
};

struct RenderSkinnedMeshInfo
{
	int	aniBoneIndex = -1;
};

struct RenderLightInfo
{
	physx::PxVec3	color = { 0.5f, 0.5f, 0.5f };
	physx::PxVec3	falloffAndPower = { 1.0f, 10.0f, 50.0f };
	float			angle = 0.0f;
	LIGHT_TYPE		type = LIGHT_TYPE::LIGHT_TYPE_COUNT;
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
		, boundSphereRad(1.0f)
	{
		lightInfo = {};
	}

	RenderInfo(const RenderInfo& src)
	{
		std::memcpy(&lightInfo, &src.lightInfo, sizeof(RenderLightInfo));
		type = src.type;
		world = src.world;
		scale = src.scale;
		boundSphereRad = src.boundSphereRad;
		meshOrTextureName = src.meshOrTextureName;
	}

	~RenderInfo()
	{

	}

	RenderInfo& operator=(const RenderInfo& src)
	{
		std::memcpy(&lightInfo, &src.lightInfo, sizeof(RenderLightInfo));
		type = src.type;
		world = src.world;
		scale = src.scale;
		boundSphereRad = src.boundSphereRad;
		meshOrTextureName = src.meshOrTextureName;

		return *this;
	}

	RENDER_TYPE		type;
	physx::PxMat44	world;
	physx::PxVec3	scale;
	float			boundSphereRad;
	std::string		meshOrTextureName;

	union
	{
		RenderSkinnedMeshInfo	skin;
		RenderPointInfo			point;
		RenderUIInfo			uiInfo;
		RenderLightInfo			lightInfo;
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
	physx::PxVec3	specular = { 0.01f,0.01f,0.01f };
	float			specularExponent = 0.25f;
	physx::PxVec3	emissive = { 0,0,0 };
	float			pad0;
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
	unsigned int	GetTotalVertexNum() const;
	unsigned int	GetStartVertexOffset() const;
	unsigned int	GetTotalIndexNum() const;
	unsigned int	GetStartIndexOffset() const;
	bool			IsOneSub() const { return subs.size() == 1; }

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