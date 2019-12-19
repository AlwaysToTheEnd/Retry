#pragma once
#include <memory>
#include <utility>
#include <vector>
#include <string>
#include <unordered_map>
#include "BaseClass.h"
#include "Vertex.h"

// This Codes was referenced from HLSLCrossCompiler-master (xfileparser.cpp)

namespace Ani
{
	class SkinnedData;
	struct AnimBone;
	struct Subset;
	struct AniMaterial;
	struct Animation;
}

class XFileParser final
{
public:
	struct BoneWeight
	{
		unsigned int	vertexIndex = 0;
		float			weight = 0;
	};

	struct Bone
	{
		std::string TargetFrame;
		physx::PxMat44 offsetMat;
		std::vector<BoneWeight> weights;
	};

public:
	XFileParser();
	virtual ~XFileParser() {}

	bool LoadXfile(const std::string& filename,
		std::vector<SkinnedVertex>& vertices,
		std::vector<unsigned int>& indices,
		std::vector<Ani::Subset>& subsets,
		std::vector<Ani::AniMaterial>& mats,
		Ani::SkinnedData& skinInfo);

private:
	void CheckFileAttribute(std::vector<char>& fileData, std::vector<char>& uncompressed);
	void ParseDataObjectTemplate();
	void ParseDataObjectFrame(int parentIndex,
							std::vector<SkinnedVertex>& vertices,
							std::vector<unsigned int>& indices,
							std::vector<Ani::Subset>& subsets,
							std::vector<Ani::AniMaterial>& mats,
							Ani::SkinnedData& skinInfo);
	void ParseDataObjectTransformationMatrix(physx::PxMat44& pMatrix);
	void ParseDataObjectMesh(std::vector<SkinnedVertex>& vertices,
							std::vector<unsigned int>& indices,
							std::vector<Ani::Subset>& subsets,
							std::vector<Ani::AniMaterial>& mats);

	void ParseDataObjectSkinMeshHeader();
	void ParseDataObjectSkinWeights(Ani::Subset& subset);
	void ParseDataObjectMeshNormals(Ani::Subset& subset, std::vector<SkinnedVertex>& vertices);
	void ParseDataObjectMeshTextureCoords(Ani::Subset& subset, std::vector<SkinnedVertex>& vertices);
	void ParseDataObjectMeshVertexColors(Ani::Subset& subset, std::vector<SkinnedVertex>& vertices);
	void ParseDataObjectMeshMaterialList(Ani::Subset& subset, std::vector<Ani::AniMaterial>& mats);
	void ParseDataObjectMaterial(Ani::AniMaterial& mats);
	void ParseDataObjectAnimTicksPerSecond();
	void ParseDataObjectAnimationSet(Ani::SkinnedData& skinInfo);
	void ParseDataObjectAnimation(Ani::Animation& pAnim);
	void ParseDataObjectAnimationKey(Ani::AnimBone& pAnimBone);
	void ParseDataObjectTextureFilename(std::string& pName);
	void ParseUnknownDataObject();

	//! places pointer to next begin of a token, and ignores comments
	void FindNextNoneWhiteSpace();

	//! returns next parseable token. Returns empty string if no token there
	std::string GetNextToken();

	//! reads header of dataobject including the opening brace.
	//! returns false if error happened, and writes name of object
	//! if there is one
	void readHeadOfDataObject(std::string* poName = NULL);

	//! checks for closing curly brace, throws exception if not there
	void CheckForClosingBrace();

	//! checks for one following semicolon, throws exception if not there
	void CheckForSemicolon();

	//! checks for a separator char, either a ',' or a ';'
	void CheckForSeparator();

	/// tests and possibly consumes a separator char, but does nothing if there was no separator
	void TestForSeparator();

	//! reads a x file style string
	void GetNextTokenAsString(std::string& poString);

	void ReadUntilEndOfLine();

	unsigned short ReadBinWord();
	unsigned int ReadBinDWord();
	unsigned int ReadInt();
	float ReadFloat();
	DirectX::XMFLOAT2 ReadVector2();
	DirectX::XMFLOAT3 ReadVector3();
	DirectX::XMFLOAT3 ReadRGB();
	DirectX::XMFLOAT4 ReadRGBA();

	/** Throws an exception with a line number and the given text. */
	void ThrowException(const std::string& pText);

private:
	void DataRearrangement(std::vector<SkinnedVertex>& vertices,
						Ani::SkinnedData& skinInfo);

protected:
	std::vector<XFileParser::Bone>						m_Bones;
	std::unordered_map<std::string, unsigned int>		m_FrameIndex;

	unsigned int m_MajorVersion, m_MinorVersion;
	bool m_IsBinaryFormat;
	unsigned int m_BinaryFloatSize;
	unsigned int m_BinaryNumCount;

	const char* P;
	const char* End;

	unsigned int m_LineNumber;
};