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
		std::vector<unsigned int> m_Indices;
	};

	/** Helper structure representing a texture filename inside a material and its potential source */
	struct TexEntry
	{
		std::string m_Name;
		bool m_IsNormalMap; // true if the texname was specified in a NormalmapFilename tag

		TexEntry() { m_IsNormalMap = false; }
		TexEntry(const std::string& pName, bool pIsNormalMap = false)
			: m_Name(pName), m_IsNormalMap(pIsNormalMap)
		{
		}
	};

	struct Material
	{
		std::string m_Name;
		bool m_IsReference = false; // if true, mName holds a name by which the actual material can be found in the material list
		DirectX::XMFLOAT4 m_Diffuse;
		float m_SpecularExponent;
		DirectX::XMFLOAT3 m_Specular;
		DirectX::XMFLOAT3 m_Emissive;
		std::vector<TexEntry> m_Textures;
	};

	/** Helper structure to represent a bone weight */
	struct BoneWeight
	{
		unsigned int m_Vertex;
		float m_Weight;
	};

	struct Bone
	{
		std::string m_Name;
		std::vector<BoneWeight> m_Weights;
		CGH::MAT16 m_OffsetMatrix;
	};

	struct Mesh
	{
		std::vector<DirectX::XMFLOAT3> m_Positions;
		std::vector<Face> m_PosFaces;
		std::vector<DirectX::XMFLOAT3> m_Normals;
		std::vector<Face> m_NormFaces;
		unsigned int m_NumTextures = 0;
		std::vector<DirectX::XMFLOAT2> m_TexCoords[AI_MAX_NUMBER_OF_TEXTURECOORDS];
		unsigned int m_NumColorSets = 0;
		std::vector<DirectX::XMFLOAT4> m_Colors[AI_MAX_NUMBER_OF_TEXTURECOORDS];

		std::vector<unsigned int> m_FaceMaterials;
		std::vector<Material> m_Materials;

		std::vector<Bone> m_Bones;
	};

	struct Node
	{
		std::string m_Name;
		CGH::MAT16 m_TrafoMatrix;
		Node* m_Parent = nullptr;
		std::vector<Node*> m_Children;
		std::vector<Mesh*> m_Meshes;
		int	m_sameNameBoneCount = 0;

		/*void GetNodeNames(std::vector<std::string>& nameList)
		{
			nameList.push_back(m_Name);
			for (auto& it : m_Children)
			{
				it->GetNodeNames(nameList);
			}
		}

		void GetBoneNames(std::vector<std::string>& nameList)
		{
			for (auto& it : m_Meshes)
			{
				for (auto& it2 : it->m_Bones)
				{
					nameList.push_back(it2.m_Name);
				}
			}

			for (auto& it : m_Children)
			{
				it->GetBoneNames(nameList);
			}
		}

		int GetChildNodeCount()
		{
			int numCount = 1;
			for (auto& it : m_Children)
			{
				if (it != nullptr)
				{
					numCount += it->GetChildNodeCount();
				}
				else
				{
					numCount++;
				}
			}

			return numCount;
		}

		size_t GetBoneCount()
		{
			size_t boneCount = 0;

			for (auto& it : m_Meshes)
			{
				boneCount += it->m_Bones.size();
			}

			for (auto& it : m_Children)
			{
				if (it != nullptr)
				{
					boneCount += it->GetBoneCount();
				}
			}

			return boneCount;
		}*/

		Node() { m_Parent = NULL; }
		Node(Node* pParent) { m_Parent = pParent; }
		~Node()
		{
			for (auto& it : m_Children)
			{
				delete it;
			}

			for (auto& it : m_Meshes)
			{
				delete it;
			}
		}
	};

	template <typename T>
	struct TimeValue
	{
		double m_Time = 0;
		T m_Value;
	};

	struct AnimBone
	{
		std::string m_BoneName;
		std::vector<TimeValue<DirectX::XMFLOAT3>> m_PosKeys;
		std::vector<TimeValue<DirectX::XMFLOAT4>> m_RotKeys;
		std::vector<TimeValue<DirectX::XMFLOAT3>> m_ScaleKeys;
		std::vector<TimeValue<CGH::MAT16>> m_TrafoKeys;
	};

	struct Animation
	{
		std::string m_Name;
		std::vector<AnimBone> m_Anims;
	};
}

class AnimationObject :public GameObject
{
public:
	AnimationObject()
		: m_RootNode(nullptr)
		, m_AnimTicksPerSecond(0)
	{

	}

	virtual ~AnimationObject()
	{
		if (m_RootNode)
		{
			delete m_RootNode;
		}
	}
	
	virtual void Init() override;
	virtual void Update() override;

public:
	Ani::Node* m_RootNode;

	std::vector<Ani::Mesh> m_GlobalMeshes;
	std::vector<Ani::Material> m_GlobalMaterials;

	std::vector<Ani::Animation> m_Anims;
	unsigned int m_AnimTicksPerSecond;
};

