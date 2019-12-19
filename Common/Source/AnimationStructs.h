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
struct AniBoneMat;

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
		std::string name;
		unsigned int localMatIndex=0;
		std::vector<TimeValue<DirectX::XMFLOAT3>> posKeys;
		std::vector<TimeValue<DirectX::XMFLOAT4>> rotKeys;
		std::vector<TimeValue<DirectX::XMFLOAT3>> scaleKeys;
		std::vector<TimeValue<physx::PxMat44>> trafoKeys;
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
		unsigned int				BoneCount() const { return static_cast<unsigned int>(m_BoneOffsets.size()); }
		unsigned int				GetClipStartTime(const std::string& clipName) const;
		unsigned int				GetClipEndTime(const std::string& clipName) const;
		void						GetFinalTransforms(const std::string& clipName, unsigned long long timePos, AniBoneMat& finalTransforms) const;
		unsigned int				GetAnimationNum() const { return static_cast<unsigned int>(m_Animations.size()); }
		std::vector<std::string>	GetAnimationNames() const;
		
		bool						CheckAnimation(const std::string& key) const;

	private:
		void CalLocalTransformFromAnimation(const std::string& clipName, std::vector<physx::PxMat44>& LocalTransforms , unsigned long long timePos) const;

		DirectX::XMVECTOR XM_CALLCONV GetAnimationKeyOnTick(const std::vector<TimeValue<DirectX::XMFLOAT3>>& values, unsigned long long timePos) const;
		DirectX::XMVECTOR XM_CALLCONV GetAnimationKeyOnTick(const std::vector<TimeValue<DirectX::XMFLOAT4>>& values, unsigned long long timePos) const;
		DirectX::XMMATRIX XM_CALLCONV GetAnimationKeyOnTick(const std::vector<TimeValue<physx::PxMat44>>& values, unsigned long long timePos) const;

	private:
		std::vector<int>							m_FrameHierarchy;
		std::vector<unsigned int>					m_BoneOffsetsFrameIndex;
		std::vector<physx::PxMat44>						m_BoneOffsets;
		std::unordered_map<std::string, Animation>	m_Animations;
	};
}