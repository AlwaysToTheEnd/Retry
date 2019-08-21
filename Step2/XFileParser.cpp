#include "XFileParser.h"
#include "d3dUtil.h"

using namespace Ani;
using namespace DirectX;
using namespace std;

XFileParser::XFileParser(const std::string& filePath)
	:m_MajorVersion(0)
	,m_MinorVersion(0)
	,m_IsBinaryFormat(false)
	,m_BinaryNumCount(0)
	,m_BinaryFloatSize(0)
	,m_LineNumber(0)
	,P(nullptr)
	,End(nullptr)
{
	FILE* load = nullptr;
	vector<char> fileData;

	fopen_s(&load, filePath.c_str(), "r");

	if (!load)
	{
		ThrowException("fileLoad fail");
	}
	else
	{
		fseek(load, 0, SEEK_END);
		fileData.resize(ftell(load));
		fseek(load, 0, SEEK_SET);

		fread_s(fileData.data(), fileData.size(), fileData.size(), 1, load);
		fclose(load);
	}

	////////////////////////////////////////////
	P= &fileData.front();
	End = &fileData.back();

	if (strncmp(P, "xof ", 4) != 0)
		ThrowException("Header mismatch, file is not an XFile.");

	// read version. It comes in a four byte format such as "0302"
	m_MajorVersion = (unsigned int)(P[4] - 48) * 10 + (unsigned int)(P[5] - 48);
	m_MinorVersion = (unsigned int)(P[6] - 48) * 10 + (unsigned int)(P[7] - 48);

	bool compressed = false;

	// txt - pure ASCII text format
	if (strncmp(P + 8, "txt ", 4) == 0)
		m_IsBinaryFormat = false;

	// bin - Binary format
	else if (strncmp(P + 8, "bin ", 4) == 0)
		m_IsBinaryFormat = true;

	// tzip - Inflate compressed text format
	else if (strncmp(P + 8, "tzip", 4) == 0)
	{
		m_IsBinaryFormat = false;
		compressed = true;
	}
	// bzip - Inflate compressed binary format
	else if (strncmp(P + 8, "bzip", 4) == 0)
	{
		m_IsBinaryFormat = true;
		compressed = true;
	}
	else ThrowException("Unsupported xfile format");

	// float size
	m_BinaryFloatSize = (unsigned int)(P[12] - 48) * 1000
		+ (unsigned int)(P[13] - 48) * 100
		+ (unsigned int)(P[14] - 48) * 10
		+ (unsigned int)(P[15] - 48);

	if (m_BinaryFloatSize != 32 && m_BinaryFloatSize != 64)
		ThrowException("Unknown float size specified in xfile header.");

	P += 16;

	// If this is a compressed X file, apply the inflate algorithm to it
//	if (compressed)
//	{
//#ifdef ASSIMP_BUILD_NO_COMPRESSED_X
//		throw DeadlyImportError("Assimp was built without compressed X support");
//#else
//		/* ///////////////////////////////////////////////////////////////////////
//		 * COMPRESSED X FILE FORMAT
//		 * ///////////////////////////////////////////////////////////////////////
//		 *    [xhead]
//		 *    2 major
//		 *    2 minor
//		 *    4 type    // bzip,tzip
//		 *    [mszip_master_head]
//		 *    4 unkn    // checksum?
//		 *    2 unkn    // flags? (seems to be constant)
//		 *    [mszip_head]
//		 *    2 ofs     // offset to next section
//		 *    2 magic   // 'CK'
//		 *    ... ofs bytes of data
//		 *    ... next mszip_head
//		 *
//		 *  http://www.kdedevelopers.org/node/3181 has been very helpful.
//		 * ///////////////////////////////////////////////////////////////////////
//		 */
//
//		 // build a zlib stream
//		z_stream stream;
//		stream.opaque = NULL;
//		stream.zalloc = &dummy_alloc;
//		stream.zfree = &dummy_free;
//		stream.data_type = (m_IsBinaryFormat ? Z_BINARY : Z_ASCII);
//
//		// initialize the inflation algorithm
//		::inflateInit2(&stream, -MAX_WBITS);
//
//		// skip unknown data (checksum, flags?)
//		P += 6;
//
//		// First find out how much storage we'll need. Count sections.
//		const char* P1 = P;
//		unsigned int est_out = 0;
//
//		while (P1 + 3 < End)
//		{
//			// read next offset
//			uint16_t ofs = *((uint16_t*)P1);
//			AI_SWAP2(ofs); P1 += 2;
//
//			if (ofs >= MSZIP_BLOCK)
//				throw DeadlyImportError("X: Invalid offset to next MSZIP compressed block");
//
//			// check magic word
//			uint16_t magic = *((uint16_t*)P1);
//			AI_SWAP2(magic); P1 += 2;
//
//			if (magic != MSZIP_MAGIC)
//				throw DeadlyImportError("X: Unsupported compressed format, expected MSZIP header");
//
//			// and advance to the next offset
//			P1 += ofs;
//			est_out += MSZIP_BLOCK; // one decompressed block is 32786 in size
//		}
//
//		// Allocate storage and terminating zero and do the actual uncompressing
//		uncompressed.resize(est_out + 1);
//		char* out = &uncompressed.front();
//		while (P + 3 < End)
//		{
//			uint16_t ofs = *((uint16_t*)P);
//			AI_SWAP2(ofs);
//			P += 4;
//
//			// push data to the stream
//			stream.next_in = (Bytef*)P;
//			stream.avail_in = ofs;
//			stream.next_out = (Bytef*)out;
//			stream.avail_out = MSZIP_BLOCK;
//
//			// and decompress the data ....
//			int ret = ::inflate(&stream, Z_SYNC_FLUSH);
//			if (ret != Z_OK && ret != Z_STREAM_END)
//				throw DeadlyImportError("X: Failed to decompress MSZIP-compressed data");
//
//			::inflateReset(&stream);
//			::inflateSetDictionary(&stream, (const Bytef*)out, MSZIP_BLOCK - stream.avail_out);
//
//			// and advance to the next offset
//			out += MSZIP_BLOCK - stream.avail_out;
//			P += ofs;
//		}
//
//		// terminate zlib
//		::inflateEnd(&stream);
//
//		// ok, update pointers to point to the uncompressed file data
//		P = &uncompressed[0];
//		End = out;
//
//		// FIXME: we don't need the compressed data anymore, could release
//		// it already for better memory usage. Consider breaking const-co.
//		DefaultLogger::get()->info("Successfully decompressed MSZIP-compressed file");
//#endif // !! ASSIMP_BUILD_NO_COMPRESSED_X
//	}
//	else
//	{
//		// start reading here
//		ReadUntilEndOfLine();
//	}
//
//	mScene = new Scene;
//	ParseFile();

	// filter the imported hierarchy for some degenerated cases
	/*if (mScene->mRootNode) {
		FilterHierarchy(mScene->mRootNode);
	}*/
}

XFileParser::~XFileParser()
{
}

void XFileParser::ParseFile()
{

}

void XFileParser::ParseDataObjectTemplate()
{
}

void XFileParser::ParseDataObjectFrame(Ani::Node* pParent)
{
}

void XFileParser::ParseDataObjectTransformationMatrix(CGH::MAT16& pMatrix)
{
}

void XFileParser::ParseDataObjectMesh(Ani::Mesh* pMesh)
{
}

void XFileParser::ParseDataObjectSkinWeights(Ani::Mesh* pMesh)
{
}

void XFileParser::ParseDataObjectSkinMeshHeader(Ani::Mesh* pMesh)
{
}

void XFileParser::ParseDataObjectMeshNormals(Ani::Mesh* pMesh)
{
}

void XFileParser::ParseDataObjectMeshTextureCoords(Ani::Mesh* pMesh)
{
}

void XFileParser::ParseDataObjectMeshVertexColors(Ani::Mesh* pMesh)
{
}

void XFileParser::ParseDataObjectMeshMaterialList(Ani::Mesh* pMesh)
{
}

void XFileParser::ParseDataObjectMaterial(Ani::Material* pMaterial)
{
}

void XFileParser::ParseDataObjectAnimTicksPerSecond()
{
}

void XFileParser::ParseDataObjectAnimationSet()
{
}

void XFileParser::ParseDataObjectAnimation(Ani::Animation* pAnim)
{
}

void XFileParser::ParseDataObjectAnimationKey(Ani::AnimBone* pAnimBone)
{
}

void XFileParser::ParseDataObjectTextureFilename(std::string& pName)
{
}

void XFileParser::ParseUnknownDataObject()
{
}

void XFileParser::FindNextNoneWhiteSpace()
{
}

std::string XFileParser::GetNextToken()
{
	return std::string();
}

void XFileParser::readHeadOfDataObject(std::string* poName)
{
}

void XFileParser::CheckForClosingBrace()
{
}

void XFileParser::CheckForSemicolon()
{
}

void XFileParser::CheckForSeparator()
{
}

void XFileParser::TestForSeparator()
{
}

void XFileParser::GetNextTokenAsString(std::string& poString)
{
}

void XFileParser::ReadUntilEndOfLine()
{
}

unsigned short XFileParser::ReadBinWord()
{
	return 0;
}

unsigned int XFileParser::ReadBinDWord()
{
	return 0;
}

unsigned int XFileParser::ReadInt()
{
	return 0;
}

float XFileParser::ReadFloat()
{
	return 0.0f;
}

DirectX::XMFLOAT2 XFileParser::ReadVector2()
{
	return DirectX::XMFLOAT2();
}

DirectX::XMFLOAT3 XFileParser::ReadVector3()
{
	return DirectX::XMFLOAT3();
}

DirectX::XMFLOAT3 XFileParser::ReadRGB()
{
	return DirectX::XMFLOAT3();
}

DirectX::XMFLOAT4 XFileParser::ReadRGBA()
{
	return DirectX::XMFLOAT4();
}

void XFileParser::ThrowException(const std::string& pText)
{
}

void XFileParser::FilterHierarchy(Ani::Node* pNode)
{
}


