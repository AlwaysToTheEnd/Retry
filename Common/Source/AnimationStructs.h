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

class MeshReplacer;
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
		unsigned int primitiveType = 0;
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
		physx::PxVec4 diffuse = { 1.0f,1.0f,1.0f,1.0f };
		float specularExponent = 0.25f;
		physx::PxVec3 specular = { 0.01f,0.01f,0.01f };
		physx::PxVec3 emissive = { 0,0,0 };
		std::vector<TexEntry> textures;
	};

	/** Helper structure to represent a bone weight */

	template <typename T>
	struct TimeValue
	{
		double time = 0;
		T value;
	};

	struct AnimBone
	{
		std::string name;
		unsigned int localMatIndex=0;
		std::vector<TimeValue<physx::PxVec3>> posKeys;
		std::vector<TimeValue<physx::PxVec3>> scaleKeys;
		std::vector<TimeValue<physx::PxVec4>> rotKeys;
		std::vector<TimeValue<physx::PxMat44>> trafoKeys;
	};

	struct Animation
	{
		double					tickPerSecond = 0.0l;
		std::vector<AnimBone>	animBones;
	};

	class SkinnedData
	{
	public:
		friend MeshReplacer;

	public:
		unsigned int				BoneCount() const { return static_cast<unsigned int>(m_BoneOffsets.size()); }
		double						GetClipStartTime(const std::string& clipName) const;
		double						GetClipEndTime(const std::string& clipName) const;
		void						GetFinalTransforms(const std::string& clipName, double timePos, AniBoneMat& finalTransforms) const;
		unsigned int				GetAnimationNum() const { return static_cast<unsigned int>(m_Animations.size()); }
		void						GetAnimationNames(std::vector<std::string>& out) const;
		
		bool						CheckAnimation(const std::string& key) const;

	private:
		void CalLocalTransformFromAnimation(const std::string& clipName, std::vector<physx::PxMat44>& LocalTransforms , double timePos) const;

		DirectX::XMVECTOR XM_CALLCONV GetAnimationKeyOnTick(const std::vector<TimeValue<DirectX::XMFLOAT3>>& values, double timePos) const;
		DirectX::XMVECTOR XM_CALLCONV GetAnimationKeyOnTick(const std::vector<TimeValue<DirectX::XMFLOAT4>>& values, double timePos) const;
		DirectX::XMMATRIX XM_CALLCONV GetAnimationKeyOnTick(const std::vector<TimeValue<physx::PxMat44>>& values, double timePos) const;

	private:
		std::vector<int>							m_FrameHierarchy;
		std::vector<unsigned int>					m_BoneOffsetsFrameIndex;
		std::vector<physx::PxMat44>					m_FrameNodesTransform;
		std::vector<physx::PxMat44>					m_BoneOffsets;
		std::unordered_map<std::string, Animation>	m_Animations;
	};
}