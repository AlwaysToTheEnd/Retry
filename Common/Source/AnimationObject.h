#pragma once
#include "GameObject.h"
#include "BaseClass.h"
#include <string>
#include <unordered_map>

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

	struct AniMaterial
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
		unsigned int m_NumTextures = 0;
		std::vector<DirectX::XMFLOAT2> m_TexCoords[AI_MAX_NUMBER_OF_TEXTURECOORDS];
		unsigned int m_NumColorSets = 0;
		std::vector<DirectX::XMFLOAT4> m_Colors[AI_MAX_NUMBER_OF_TEXTURECOORDS];

		std::vector<unsigned int> m_FaceMaterials;
		std::vector<AniMaterial> m_Materials;

		std::vector<Bone>			m_Bones;
		std::vector<CGH::MAT16*>	m_BoneMatrixPtrs;
		std::vector<CGH::MAT16>		m_CurrentBoneMatrices;
	};

	struct Node
	{
		std::string m_Name;
		CGH::MAT16 m_CombinedTransformationMatrix;
		CGH::MAT16 m_TransformMat;
		Node* m_Parent = nullptr;
		std::vector<Node*> m_Children;
		std::vector<Mesh*> m_Meshes;

		Node() { m_Parent = NULL; }
		Node(Node* pParent) { m_Parent = pParent; }
		~Node()
		{
			for (auto& it : m_Children)
			{
				delete it;
			}
		}

		void GetTransformMats(std::vector<CGH::MAT16>& out)
		{
			out.push_back(m_TransformMat);

			for (auto& it : m_Children)
			{
				it->GetTransformMats(out);
			}
		}

		Node* SearchNodeByName(const std::string& name)
		{
			Node* result = nullptr;

			for (auto& it : m_Children)
			{
				result = it->SearchNodeByName(name);

				if (result)
				{
					return result;
				}
			}

			if (m_Name == name)
			{
				result = this;
			}

			return result;
		}

		Node* SearchNodeByMesh(const Mesh* const meshPtr)
		{
			Node* result = nullptr;

			for (auto& it : m_Children)
			{
				result = it->SearchNodeByMesh(meshPtr);

				if (result)
				{
					break;
				}
			}

			for (auto& it : m_Meshes)
			{
				if (it == meshPtr)
				{
					result = this;
					break;
				}
			}

			return result;
		}
	};

	template <typename T>
	struct TimeValue
	{
		unsigned int m_Time = 0;
		T m_Value;
	};

	struct AnimBone
	{
		std::string	m_BoneName;
		std::vector<TimeValue<DirectX::XMFLOAT3>> m_PosKeys;
		std::vector<TimeValue<DirectX::XMFLOAT4>> m_RotKeys;
		std::vector<TimeValue<DirectX::XMFLOAT3>> m_ScaleKeys;
		std::vector<TimeValue<CGH::MAT16>> m_TrafoKeys;

		bool IsMatrixDataType()
		{
			if (m_PosKeys.size())
			{
				return false;
			}
			else if (m_TrafoKeys.size())
			{
				return true;
			}

			assert(false);
			return true;
		}
	};

	struct Animation
	{
		std::vector<AnimBone> m_AnimBones;
	};
}

class AnimationObject :public GameObject
{
public:
	AnimationObject()
		: m_RootNode(nullptr)
		, m_AnimTicksPerSecond(0)
	{
		m_CurrAnimation = m_Anims.end();
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
	bool SetAnimation(std::string name);
	void SetAnimationTime(unsigned int tickTime) { m_AnimTicksPerSecond = tickTime; }

	void GetAnimationNames(std::vector<std::string>& out);
	std::string GetCurrAniName();

private:
	void SetupBoneMatrixPtrs(Ani::Node* node);
	void Update(Ani::Node* node);
	void UpdateSkinnedMesh(Ani::Node* node);
	void CalFinalTransform();

	template<typename T>
	DirectX::XMVECTOR GetAnimationKeyOnTick(const std::vector<Ani::TimeValue<T>>& values);

public:
	Ani::Node* m_RootNode;

	std::vector<std::unique_ptr<Ani::Mesh>>			m_GlobalMeshes;
	std::vector<std::unique_ptr<Ani::AniMaterial>>	m_GlobalMaterials;

	/////////
	std::unordered_map<std::string, std::unique_ptr<Ani::Animation>>			m_Anims;
	std::unordered_map<std::string, std::unique_ptr<Ani::Animation>>::iterator	m_CurrAnimation;

	std::unordered_map<std::string, DirectX::XMFLOAT4X4*>						m_TransformationMatPtrs;
	unsigned int																m_AnimTicksPerSecond;
};

template<typename T>
inline DirectX::XMVECTOR AnimationObject::GetAnimationKeyOnTick(const std::vector<Ani::TimeValue<T>>& values)
{
	DirectX::XMVECTOR result = DirectX::XMVectorSet(0, 0, 0, 1);

	if (m_AnimTicksPerSecond <= values.front().m_Time)
	{
		result = DirectX::XMLoadFloat3(&values.front().m_Value);
	}
	else if (m_AnimTicksPerSecond >= values.back().m_Time)
	{
		result = DirectX::XMLoadFloat3(&values.back().m_Value);
	}
	else
	{
		for (size_t i = 0; i < values.size() - 1; i++)
		{
			if (m_AnimTicksPerSecond >= values[i].m_Time && m_AnimTicksPerSecond <= values[i + 1].m_Time)
			{
				float lerpPercent =
					(m_AnimTicksPerSecond - values[i].m_Time) /
					(values[i + 1].m_Time - values[i].m_Time);

				DirectX::XMVECTOR prev = DirectX::XMLoadFloat3(&values[i].m_Value);
				DirectX::XMVECTOR next = DirectX::XMLoadFloat3(&values[i+1].m_Value);
			
				result = DirectX::XMVectorLerp(prev, next, lerpPercent);

				break;
			}
		}
	}

	return result;
}

template<>
inline DirectX::XMVECTOR AnimationObject::GetAnimationKeyOnTick(const std::vector<Ani::TimeValue<DirectX::XMFLOAT4>>& values)
{
	DirectX::XMVECTOR result = DirectX::XMVectorSet(0, 0, 0, 1);

	if (m_AnimTicksPerSecond <= values.front().m_Time)
	{
		result = DirectX::XMLoadFloat4(&values.front().m_Value);
	}
	else if (m_AnimTicksPerSecond >= values.back().m_Time)
	{
		result = DirectX::XMLoadFloat4(&values.back().m_Value);
	}
	else
	{
		for (size_t i = 0; i < values.size() - 1; i++)
		{
			if (m_AnimTicksPerSecond >= values[i].m_Time && m_AnimTicksPerSecond <= values[i + 1].m_Time)
			{
				float lerpPercent =
					(m_AnimTicksPerSecond - values[i].m_Time) /
					(values[i + 1].m_Time - values[i].m_Time);

				DirectX::XMVECTOR prev = DirectX::XMLoadFloat4(&values[i].m_Value);
				DirectX::XMVECTOR next = DirectX::XMLoadFloat4(&values[i + 1].m_Value);

				result = DirectX::XMQuaternionSlerp(prev, next, lerpPercent);

				break;
			}
		}
	}

	return result;
}

