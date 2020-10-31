#pragma once
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include "assimp/scene.h"
#include "BaseClass.h"
#include "Vertex.h"

namespace Ani
{
	class SkinnedData;
	struct AnimBone;
	struct Subset;
	struct AniMaterial;
	struct Animation;
}

class MeshReplacer final
{

public:
	MeshReplacer();
	virtual ~MeshReplacer() {}

	bool Replace(const aiScene* assimpData,
				std::vector<SkinnedVertex>& vertices,
				std::vector<unsigned int>& indices,
				std::vector<Ani::Subset>& subsets,
				std::vector<Ani::AniMaterial>& mats,
				Ani::SkinnedData& skinInfo);
private:
	void ReplaceMaterialData(const aiScene* assimpData, std::vector<Ani::AniMaterial>& mats);
	void ReplaceMeshData(const aiScene* assimpData, 
						std::vector<Ani::Subset>& subsets,
						std::vector<SkinnedVertex>& vertices, 
						std::vector<unsigned int>& indices,
						std::vector<Ani::AniMaterial>& mats);
	void ReplaceFrameHierarchy(const aiNode* node, int parent, Ani::SkinnedData& skinInfo);
	void ReplaceBoneData(const aiScene* assimpData,
						std::vector<SkinnedVertex>& vertices,
						std::vector<unsigned int>& indices,
						Ani::SkinnedData& skinInfo);
	void ReplaceAnimationData(const aiScene* assimpData, Ani::SkinnedData& skinInfo);

private:
	std::unordered_map<std::string, unsigned int> m_FramesIndex;
};
