#pragma once
#include "GameObject.h"
#include "BaseClass.h"
#include <string>

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

namespace Ani
{
	struct Face
	{
		std::vector<unsigned int> mIndices;
	};

	/** Helper structure representing a texture filename inside a material and its potential source */
	struct TexEntry
	{
		std::string mName;
		bool mIsNormalMap; // true if the texname was specified in a NormalmapFilename tag

		TexEntry() { mIsNormalMap = false; }
		TexEntry(const std::string& pName, bool pIsNormalMap = false)
			: mName(pName), mIsNormalMap(pIsNormalMap)
		{
		}
	};

	struct Material
	{
		std::string mName;
		bool mIsReference=false; // if true, mName holds a name by which the actual material can be found in the material list
		DirectX::XMFLOAT4 mDiffuse;
		float mSpecularExponent;
		DirectX::XMFLOAT3 mSpecular;
		DirectX::XMFLOAT3 mEmissive;
		std::vector<TexEntry> mTextures;
	};

	/** Helper structure to represent a bone weight */
	struct BoneWeight
	{
		unsigned int mVertex;
		float mWeight;
	};

	struct Bone
	{
		std::string mName;
		std::vector<BoneWeight> mWeights;
		CGH::MAT16 mOffsetMatrix;
	};

	struct Mesh
	{
		std::vector<DirectX::XMFLOAT3> mPositions;
		std::vector<Face> mPosFaces;
		std::vector<DirectX::XMFLOAT3> mNormals;
		std::vector<Face> mNormFaces;
		unsigned int mNumTextures;
		std::vector<DirectX::XMFLOAT2> mTexCoords[8];
		unsigned int mNumColorSets;
		std::vector<DirectX::XMFLOAT4> mColors[8];

		std::vector<unsigned int> mFaceMaterials;
		std::vector<Material> mMaterials;

		std::vector<Bone> mBones;

		Mesh() { mNumTextures = 0; mNumColorSets = 0; }
	};

	struct Node
	{
		std::string mName;
		CGH::MAT16 mTrafoMatrix;
		Node* mParent;
		std::vector<Node*> mChildren;
		std::vector<Mesh*> mMeshes;

		Node() { mParent = NULL; }
		Node(Node* pParent) { mParent = pParent; }
	};

	template <typename T>
	struct TimeValue
	{
		double m_Time = 0;
		T m_Value;
	};

	struct AnimBone
	{

		std::string mBoneName;
		std::vector<TimeValue<DirectX::XMFLOAT3>> mPosKeys;
		std::vector<TimeValue<DirectX::XMFLOAT4>> mRotKeys;
		std::vector<TimeValue<DirectX::XMFLOAT3>> mScaleKeys;
		std::vector<TimeValue<CGH::MAT16>> mTrafoKeys;
	};

	struct Animation
	{
		std::string mName;
		std::vector<std::unique_ptr<AnimBone>> mAnims;
	};
}

class AnimationObject :public GameObject
{
public:
	AnimationObject();
	virtual ~AnimationObject();

	virtual void Init() override;
	virtual void Update() override;

public:
	Ani::Node* mRootNode;

	std::vector<Ani::Mesh*> mGlobalMeshes;
	std::vector<Ani::Material> mGlobalMaterials;

	std::vector<Ani::Animation*> mAnims;
	unsigned int mAnimTicksPerSecond;
};

