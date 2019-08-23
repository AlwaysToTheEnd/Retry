#pragma once

#include <memory>
#include <string>
#include <vector>
#include "AnimationObject.h"

namespace Ani
{
	struct Mesh;
	struct Node;
	struct AnimBone;
	struct Animation;
	struct Material;
}

class XFileParser final
{
public:
	XFileParser() = delete;
	XFileParser(const std::string& filePath);

	~XFileParser();

	std::unique_ptr<AnimationObject> GetAniObject() { return move(m_AniObject); }

private:
	void ParseFile();
	void ParseDataObjectTemplate();
	void ParseDataObjectFrame(Ani::Node* pParent);
	void ParseDataObjectTransformationMatrix(CGH::MAT16& pMatrix);
	void ParseDataObjectMesh(Ani::Mesh* pMesh);
	void ParseDataObjectSkinWeights(Ani::Mesh* pMesh);
	void ParseDataObjectSkinMeshHeader(Ani::Mesh* pMesh);
	void ParseDataObjectMeshNormals(Ani::Mesh* pMesh);
	void ParseDataObjectMeshTextureCoords(Ani::Mesh* pMesh);
	void ParseDataObjectMeshVertexColors(Ani::Mesh* pMesh);
	void ParseDataObjectMeshMaterialList(Ani::Mesh* pMesh);
	void ParseDataObjectMaterial(Ani::Material* pMaterial);
	void ParseDataObjectAnimTicksPerSecond();
	void ParseDataObjectAnimationSet();
	void ParseDataObjectAnimation(Ani::Animation* pAnim);
	void ParseDataObjectAnimationKey(Ani::AnimBone* pAnimBone);
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

	/** Filters the imported hierarchy for some degenerated cases that some exporters produce.
	 * @param pData The sub-hierarchy to filter
	 */
	void FilterHierarchy(Ani::Node* pNode);

protected:
	unsigned int m_MajorVersion, m_MinorVersion;
	bool m_IsBinaryFormat;
	unsigned int m_BinaryFloatSize;
	unsigned int m_BinaryNumCount;

	const char* P;
	const char* End;

	unsigned int m_LineNumber;

private:
	std::unique_ptr<AnimationObject> m_AniObject;
};

