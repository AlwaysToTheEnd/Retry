#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include "BaseClass.h"
#include "Vertex.h"

//Maximum number of indices per face (polygon). 
#define AI_MAX_FACE_INDICES 0x7fff

//Maximum number of indices per face (polygon).
#define AI_MAX_BONE_WEIGHTS 0x7fffffff

//Maximum number of vertices per mesh.
#define AI_MAX_VERTICES 0x7fffffff

//Maximum number of faces per mesh.
#define AI_MAX_FACES 0x7fffffff

//Supported number of vertex color sets per mesh.
#define AI_MAX_NUMBER_OF_COLOR_SETS 0x8

//Supported number of texture coord sets (UV(W) channels) per mesh
#define AI_MAX_NUMBER_OF_TEXTURECOORDS 0x8

class XFileParser;

namespace Ani
{
	struct Subset
	{
		unsigned int vertexStart = 0;
		unsigned int vertexCount = 0;
		unsigned int indexStart = 0;
		unsigned int indexCount = 0;
		unsigned int numTexture = 0;
		unsigned int numColors = 0;
		std::vector<std::pair<std::string, unsigned int>> materialIndexCount;
	};

	struct Face
	{
		std::vector<unsigned int> indices;
	};

	struct TexEntry
	{
		std::string name;
		bool isNormalMap; // true if the texname was specified in a NormalmapFilename tag

		TexEntry() { isNormalMap = false; }
		TexEntry(const std::string& pName, bool pIsNormalMap = false)
			: name(pName), isNormalMap(pIsNormalMap)
		{
		}
	};

	struct AniMaterial
	{
		std::string name;
		bool isReference = false; // if true, mName holds a name by which the actual material can be found in the material list
		DirectX::XMFLOAT4 diffuse;
		float specularExponent;
		DirectX::XMFLOAT3 specular;
		DirectX::XMFLOAT3 emissive;
		std::vector<TexEntry> textures;
	};

	/** Helper structure to represent a bone weight */

	template <typename T>
	struct TimeValue
	{
		unsigned int time = 0;
		T value;
	};

	struct AnimBone
	{
		unsigned int localMatIndex=0;
		std::vector<TimeValue<DirectX::XMFLOAT3>> posKeys;
		std::vector<TimeValue<DirectX::XMFLOAT4>> rotKeys;
		std::vector<TimeValue<DirectX::XMFLOAT3>> scaleKeys;
		std::vector<TimeValue<CGH::MAT16>> trafoKeys;
	};

	struct Animation
	{
		std::vector<AnimBone> animBones;
	};

	class SkinnedData
	{
	public:
		friend XFileParser;

	public:
		unsigned int BoneCount() const { return static_cast<unsigned int>(m_BoneOffsets.size()); }
		float GetClipStartTime(const std::string& clipName) const;
		float GetClipEndTime(const std::string& clipName) const;
		void GetFinalTransforms(const std::string& clipName, float timePos,
			std::vector<CGH::MAT16>& finalTransforms);
		unsigned int GetAnimationNum() const { return static_cast<unsigned int>(m_Animations.size()); }

	private:
		void CalLocalTransformFromAnimation(const std::string& clipName, float timePos);

		DirectX::XMVECTOR XM_CALLCONV GetAnimationKeyOnTick(const std::vector<TimeValue<DirectX::XMFLOAT3>>& values, float timePos) const;
		DirectX::XMVECTOR XM_CALLCONV GetAnimationKeyOnTick(const std::vector<TimeValue<DirectX::XMFLOAT4>>& values, float timePos) const;

	private:
		std::vector<int>							m_BoneHierarchy;
		std::vector<CGH::MAT16>						m_BoneOffsets;
		std::vector<CGH::MAT16>						m_LocalTrnasform;
		std::unordered_map<std::string, Animation>	m_Animations;
	};
}

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
	CGH::MESH_TYPE	type = CGH::MESH_NORMAL;

	std::unordered_map<std::string, SubmeshData> subs;
};