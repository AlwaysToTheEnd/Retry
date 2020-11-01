#include "MeshReplacer.h"
#include "d3dUtil.h"
#include "AnimationStructs.h"
#include "assimp/postprocess.h"

MeshReplacer::MeshReplacer()
{

}

bool MeshReplacer::Replace(const aiScene* assimpData, std::vector<SkinnedVertex>& vertices, std::vector<unsigned int>& indices, std::vector<Ani::Subset>& subsets, std::vector<Ani::AniMaterial>& mats, Ani::SkinnedData& skinInfo)
{
	vertices.clear();
	indices.clear();
	subsets.clear();
	mats.clear();
	skinInfo.m_Animations.clear();
	skinInfo.m_FrameHierarchy.clear();
	skinInfo.m_BoneOffsets.clear();
	skinInfo.m_BoneOffsetsFrameIndex.clear();
	m_FramesIndex.clear();
	
	ReplaceMaterialData(assimpData, mats);
	ReplaceMeshData(assimpData, subsets, vertices, indices, mats);
	ReplaceFrameHierarchy(assimpData->mRootNode, -1, skinInfo);
	ReplaceBoneData(assimpData, vertices, indices, skinInfo);
	ReplaceAnimationData(assimpData, skinInfo);

	return true;
}

void MeshReplacer::ReplaceMaterialData(const aiScene* assimpData, std::vector<Ani::AniMaterial>& mats)
{
	unsigned int numMaterials = assimpData->mNumMaterials;

	for (unsigned int i = 0; i < numMaterials; i++)
	{
		aiMaterial* currMaterial = assimpData->mMaterials[i];
		Ani::AniMaterial materTemp;
		
		materTemp.name = currMaterial->GetName().C_Str();
		
		for (int type = aiTextureType_UNKNOWN; type >= 0; type--)
		{
			unsigned int textureCount = currMaterial->GetTextureCount(aiTextureType(type));
			
			while (textureCount)
			{
				aiString temp;
				Ani::TexEntry texTemp;
				
				aiReturn result = currMaterial->GetTexture(aiTextureType(type), textureCount-1, &temp);
				assert(result == aiReturn_SUCCESS);

				std::string extension;
				texTemp.name = GetFileNameFromPath(std::string(temp.C_Str()), extension);
				texTemp.isNormalMap = aiTextureType_NORMALS == aiTextureType(type);
				materTemp.textures.push_back(texTemp);

				textureCount--;
			}
		}

		mats.push_back(materTemp);
	}
}

void MeshReplacer::ReplaceMeshData(const aiScene* assimpData, 
									std::vector<Ani::Subset>& subsets,
									std::vector<SkinnedVertex>& vertices, 
									std::vector<unsigned int>& indices,
									std::vector<Ani::AniMaterial>& mats)
{
	unsigned int accumVertices = 0;
	unsigned int accumIndices = 0;

	for (unsigned int i = 0; i < assimpData->mNumMeshes; i++)
	{
		aiMesh* currMesh = assimpData->mMeshes[i];
		
		SkinnedVertex tempVertex;
		Ani::Subset tempSub;
		
		const unsigned int numVertices = currMesh->mNumVertices;
		if (currMesh->HasNormals())
		{
			//Fill Position, Normal
			for (unsigned int j = 0; j < numVertices; j++)
			{
				tempVertex.position = physx::PxVec3(currMesh->mVertices[j].x, currMesh->mVertices[j].y, currMesh->mVertices[j].z);
				tempVertex.normal = physx::PxVec3(currMesh->mNormals[j].x, currMesh->mNormals[j].y, currMesh->mNormals[j].z);
				vertices.push_back(tempVertex);
			}
		}
		else
		{
			//Fill Position
			for (unsigned int j = 0; j < numVertices; j++)
			{
				tempVertex.position = physx::PxVec3(currMesh->mVertices[j].x, currMesh->mVertices[j].y, currMesh->mVertices[j].z);
				vertices.push_back(tempVertex);
			}
		}

		if (currMesh->HasTextureCoords(0))
		{
			//Fill UV
			aiVector3D* currTexCoords = currMesh->mTextureCoords[0];
			for (unsigned int j = 0; j < numVertices; j++)
			{
				vertices[static_cast<UINT64>(j)+ accumVertices].uv= physx::PxVec2(currTexCoords[j].x, currTexCoords[j].y);
			}
		}

		//Change vertex weight ID of bone to different standard
		const unsigned int numBone = currMesh->mNumBones;
		for (unsigned int j = 0; j < numBone; j++)
		{
			aiBone* currBone = currMesh->mBones[j];

			for (unsigned int k = 0; k < currBone->mNumWeights; k++)
			{
				currBone->mWeights[k].mVertexId += accumVertices;
			}
		}

		//Fill Index
		const unsigned int numFace = currMesh->mNumFaces;
		unsigned int numIndices = 0;
		
		for (unsigned int j = 0; j < numFace; j++)
		{
			aiFace& currFace = currMesh->mFaces[j];
			numIndices += currFace.mNumIndices;

			for (unsigned int k = 0; k < currFace.mNumIndices; k++)
			{
				indices.push_back(currFace.mIndices[k] + accumVertices);
			}
		}
		
		//Fill Subset
		tempSub.materialIndexCount.push_back({ mats[currMesh->mMaterialIndex].name, numIndices });
		tempSub.primitiveType = currMesh->mPrimitiveTypes;
		tempSub.vertexCount = numVertices;
		tempSub.vertexStart = accumVertices;
		tempSub.indexCount = numIndices;
		tempSub.indexStart = accumIndices;
		tempSub.numTexture = CGH::SizeTTransUINT(mats[currMesh->mMaterialIndex].textures.size());
		subsets.push_back(tempSub);

		accumVertices += numVertices;
		accumIndices += numIndices;
	}
}

void MeshReplacer::ReplaceFrameHierarchy(const aiNode* node, int parent, Ani::SkinnedData& skinInfo)
{
	unsigned int currFrameIndex = CGH::SizeTTransUINT(skinInfo.m_FrameHierarchy.size());
	std::string nodeName = node->mName.C_Str();
	skinInfo.m_FrameNodesTransform.emplace_back();
	memcpy(&skinInfo.m_FrameNodesTransform.back(), &node->mTransformation.a1, sizeof(physx::PxMat44));
	skinInfo.m_FrameNodesTransform.back() = skinInfo.m_FrameNodesTransform.back().getTranspose();

	m_NodeNames.push_back({ nodeName, currFrameIndex });

	assert(m_FramesIndex.find(nodeName) == m_FramesIndex.end());
	m_FramesIndex.insert({ nodeName,currFrameIndex });
	skinInfo.m_FrameHierarchy.push_back(parent);
	
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		ReplaceFrameHierarchy(node->mChildren[i], currFrameIndex, skinInfo);
	}
}

void MeshReplacer::ReplaceBoneData(const aiScene* assimpData, 
								   std::vector<SkinnedVertex>& vertices, 
								   std::vector<unsigned int>& indices, 
								   Ani::SkinnedData& skinInfo)
{
	unsigned int currBoneID = 0;
	aiBone* prevMeshsFirstBone = nullptr;
	std::vector<unsigned int> vertexWeightCount(vertices.size());

	for (unsigned int i = 0; i < assimpData->mNumMeshes; i++)
	{
		aiMesh* currMesh = assimpData->mMeshes[i];

		unsigned int numBone = currMesh->mNumBones;
		for (unsigned int j = 0; j < numBone; j++)
		{
			aiBone* currBone = currMesh->mBones[j];
			auto indexIter = m_FramesIndex.find(currBone->mName.C_Str());

			assert(indexIter != m_FramesIndex.end());

			skinInfo.m_BoneOffsets.push_back(physx::PxMat44(&currBone->mOffsetMatrix.Transpose().a1));
			skinInfo.m_BoneOffsetsFrameIndex.push_back(indexIter->second);

			//Fill Bone weight datas
			for (unsigned int k = 0; k < currBone->mNumWeights; k++)
			{
				unsigned int targetIndex = currBone->mWeights[k].mVertexId;
				vertices[targetIndex].boneWeights[vertexWeightCount[targetIndex]] = currBone->mWeights[k].mWeight;
				vertices[targetIndex].boneIndices[vertexWeightCount[targetIndex]] = currBoneID;
				vertexWeightCount[targetIndex]++;
			}

			currBoneID++;
		}
	}
}

void MeshReplacer::ReplaceAnimationData(const aiScene* assimpData, Ani::SkinnedData& skinInfo)
{
	for (unsigned int i = 0; i < assimpData->mNumAnimations; i++)
	{
		aiAnimation* currAni = assimpData->mAnimations[i];
		Ani::Animation aniTemp;
		std::string aniName = currAni->mName.C_Str();
		
		aniTemp.tickPerSecond = currAni->mTicksPerSecond;
		
		for (unsigned int j = 0; j < currAni->mNumChannels; j++)
		{
			aiNodeAnim* currNode = currAni->mChannels[j];
			std::string nodeName = currNode->mNodeName.C_Str();
			auto indexIter = m_FramesIndex.find(nodeName);

			assert(indexIter != m_FramesIndex.end());
			
			aniTemp.animBones.emplace_back();
			Ani::AnimBone& aniboneTemp = aniTemp.animBones.back();

			aniboneTemp.name = nodeName;
			aniboneTemp.localMatIndex = indexIter->second;
			aniboneTemp.scaleKeys.resize(currNode->mNumScalingKeys);
			aniboneTemp.rotKeys.resize(currNode->mNumRotationKeys);
			aniboneTemp.posKeys.resize(currNode->mNumPositionKeys);

			for (unsigned int k = 0; k < currNode->mNumScalingKeys; k++)
			{
				aniboneTemp.scaleKeys[k].time = currNode->mScalingKeys[k].mTime;
				aniboneTemp.scaleKeys[k].value.x = currNode->mScalingKeys[k].mValue.x;
				aniboneTemp.scaleKeys[k].value.y = currNode->mScalingKeys[k].mValue.y;
				aniboneTemp.scaleKeys[k].value.z = currNode->mScalingKeys[k].mValue.z;
			}

			for (unsigned int k = 0; k < currNode->mNumRotationKeys; k++)
			{
				aniboneTemp.rotKeys[k].time = currNode->mRotationKeys[k].mTime;
				aniboneTemp.rotKeys[k].value.x = currNode->mRotationKeys[k].mValue.x;
				aniboneTemp.rotKeys[k].value.y = currNode->mRotationKeys[k].mValue.y;
				aniboneTemp.rotKeys[k].value.z = currNode->mRotationKeys[k].mValue.z;
				aniboneTemp.rotKeys[k].value.w = currNode->mRotationKeys[k].mValue.w;
			}

			for (unsigned int k = 0; k < currNode->mNumPositionKeys; k++)
			{
				aniboneTemp.posKeys[k].time = currNode->mPositionKeys[k].mTime;
				aniboneTemp.posKeys[k].value.x = currNode->mPositionKeys[k].mValue.x;
				aniboneTemp.posKeys[k].value.y = currNode->mPositionKeys[k].mValue.y;
				aniboneTemp.posKeys[k].value.z = currNode->mPositionKeys[k].mValue.z;
			}
		}

		assert(skinInfo.m_Animations.end() == skinInfo.m_Animations.find(aniName));
		skinInfo.m_Animations.insert({ aniName, aniTemp });
	}
}


