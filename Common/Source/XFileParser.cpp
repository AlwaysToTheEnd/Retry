#include "XFileParser.h"
#include "d3dUtil.h"
#include "zlib/zlib.h"
#include "fast_atof.h"
#include "ByteSwap.h"

using namespace Ani;
using namespace DirectX;
using namespace std;

// Magic identifier for MSZIP compressed data
#define MSZIP_MAGIC 0x4B43
#define MSZIP_BLOCK 32786

static void* dummy_alloc(void* /*opaque*/, unsigned int items, unsigned int size) {
	return ::operator new(items * size);
}

static void  dummy_free(void* /*opaque*/, void* address) {
	return ::operator delete(address);
}

XFileParser::XFileParser(const string& filePath)
	:m_MajorVersion(0)
	, m_MinorVersion(0)
	, m_IsBinaryFormat(false)
	, m_BinaryNumCount(0)
	, m_BinaryFloatSize(0)
	, m_LineNumber(0)
	, P(nullptr)
	, End(nullptr)
{
	FILE* load = nullptr;
	vector<char> fileData;
	vector<char> uncompressed;

	fopen_s(&load, filePath.c_str(), "rb");

	if (!load)
	{
		ThrowException("fileLoad fail");
	}
	else
	{
		_fseeki64(load, 0, SEEK_END);
		UINT64 dataSize = _ftelli64(load) + 1;
		fileData.resize(dataSize);
		_fseeki64(load, 0, SEEK_SET);

		fread_s(fileData.data(), dataSize, dataSize, 1, load);
		fclose(load);
	}

	////////////////////////////////////////////
	P = &fileData.front();
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
	if (compressed)
	{
#ifdef ASSIMP_BUILD_NO_COMPRESSED_X
		throw DeadlyImportError("Assimp was built without compressed X support");
#else
		/* ///////////////////////////////////////////////////////////////////////
		 * COMPRESSED X FILE FORMAT
		 * ///////////////////////////////////////////////////////////////////////
		 *    [xhead]
		 *    2 major
		 *    2 minor
		 *    4 type    // bzip,tzip
		 *    [mszip_master_head]
		 *    4 unkn    // checksum?
		 *    2 unkn    // flags? (seems to be constant)
		 *    [mszip_head]
		 *    2 ofs     // offset to next section
		 *    2 magic   // 'CK'
		 *    ... ofs bytes of data
		 *    ... next mszip_head
		 *
		 *  http://www.kdedevelopers.org/node/3181 has been very helpful.
		 * ///////////////////////////////////////////////////////////////////////
		 */

		 // build a zlib stream
		z_stream stream;
		stream.opaque = NULL;
		stream.zalloc = &dummy_alloc;
		stream.zfree = &dummy_free;
		stream.data_type = (m_IsBinaryFormat ? Z_BINARY : Z_ASCII);

		// initialize the inflation algorithm
		::inflateInit2(&stream, -MAX_WBITS);

		// skip unknown data (checksum, flags?)
		P += 6;

		// First find out how much storage we'll need. Count sections.
		const char* P1 = P;
		unsigned int est_out = 0;

		while (P1 + 3 < End)
		{
			// read next offset
			uint16_t ofs = *((uint16_t*)P1);
			AI_SWAP2(ofs); P1 += 2;

			if (ofs >= MSZIP_BLOCK)
				throw exception("X: Invalid offset to next MSZIP compressed block");

			// check magic word
			uint16_t magic = *((uint16_t*)P1);
			AI_SWAP2(magic); P1 += 2;

			if (magic != MSZIP_MAGIC)
				throw exception("X: Unsupported compressed format, expected MSZIP header");

			// and advance to the next offset
			P1 += ofs;
			est_out += MSZIP_BLOCK; // one decompressed block is 32786 in size
		}

		// Allocate storage and terminating zero and do the actual uncompressing
		uncompressed.resize(static_cast<UINT64>(est_out + 1));
		char* out = &uncompressed.front();
		while (P + 3 < End)
		{
			uint16_t ofs = *((uint16_t*)P);
			AI_SWAP2(ofs);
			P += 4;

			// push data to the stream
			stream.next_in = (Bytef*)P;
			stream.avail_in = ofs;
			stream.next_out = (Bytef*)out;
			stream.avail_out = MSZIP_BLOCK;

			// and decompress the data ....
			int ret = ::inflate(&stream, Z_SYNC_FLUSH);
			if (ret != Z_OK && ret != Z_STREAM_END)
				throw exception("X: Failed to decompress MSZIP-compressed data");

			::inflateReset(&stream);
			::inflateSetDictionary(&stream, (const Bytef*)out, MSZIP_BLOCK - stream.avail_out);

			// and advance to the next offset
			out += MSZIP_BLOCK - stream.avail_out;
			P += ofs;
		}

		// terminate zlib
		::inflateEnd(&stream);

		// ok, update pointers to point to the uncompressed file data
		P = &uncompressed[0];
		End = out;

		// FIXME: we don't need the compressed data anymore, could release
		// it already for better memory usage. Consider breaking const-co.
		OutputDebugStringA("Successfully decompressed MSZIP-compressed file");
#endif // !! ASSIMP_BUILD_NO_COMPRESSED_X
	}
	else
	{
		// start reading here
		ReadUntilEndOfLine();
	}

	m_AniObject = make_unique<AnimationObject>();

	ParseFile();

	// filter the imported hierarchy for some degenerated cases
	if (m_AniObject->m_RootNode)
	{
		FilterHierarchy(m_AniObject->m_RootNode);
	}
}

XFileParser::~XFileParser()
{
}

void XFileParser::ParseFile()
{
	bool running = true;
	while (running)
	{
		// read name of next object
		string objectName = GetNextToken();
		if (objectName.length() == 0)
		{
			break;
		}

		// parse specific object
		if (objectName == "template")
		{
			ParseDataObjectTemplate();
		}
		else if (objectName == "Frame")
		{
			ParseDataObjectFrame(NULL);
		}
		else if (objectName == "Mesh")
		{
			// some meshes have no frames at all
			m_AniObject->m_GlobalMeshes.emplace_back(new Mesh);
			ParseDataObjectMesh(m_AniObject->m_GlobalMeshes.back().get());
		}
		else if (objectName == "AnimTicksPerSecond")
		{
			ParseDataObjectAnimTicksPerSecond();
		}
		else if (objectName == "AnimationSet")
		{
			ParseDataObjectAnimationSet();
		}
		else if (objectName == "Material")
		{
			// Material outside of a mesh or node
			m_AniObject->m_GlobalMaterials.emplace_back(new Material);
			ParseDataObjectMaterial(m_AniObject->m_GlobalMaterials.back().get());
		}
		else if (objectName == "}")
		{
			OutputDebugStringA("} found in dataObject");
			// whatever?
			OutputDebugStringA("} found in dataObject");
		}
		else
		{
			// unknown format
			OutputDebugStringA("Unknown data object in animation of .x file");
			ParseUnknownDataObject();
		}
	}
}

void XFileParser::ParseDataObjectTemplate()
{
	// parse a template data object. Currently not stored.
	string name;
	readHeadOfDataObject(&name);

	// read GUID
	string guid = GetNextToken();

	// read and ignore data members
	bool running = true;
	while (running)
	{
		string s = GetNextToken();

		if (s == "}")
			break;

		if (s.length() == 0)
			ThrowException("Unexpected end of file reached while parsing template definition");
	}
}

void XFileParser::ParseDataObjectFrame(Ani::Node* pParent)
{
	// A coordinate frame, or "frame of reference." The Frame template
	// is open and can contain any object. The Direct3D extensions (D3DX)
	// mesh-loading functions recognize Mesh, FrameTransformMatrix, and
	// Frame template instances as child objects when loading a Frame
	// instance.
	string name;
	readHeadOfDataObject(&name);

	// create a named node and place it at its parent, if given
	Node* node = new Node(pParent);
	node->m_Name = name;
	if (pParent)
	{
		pParent->m_Children.push_back(node);
	}
	else
	{
		// there might be multiple root nodes
		if (m_AniObject->m_RootNode != NULL)
		{
			// place a dummy root if not there
			if (m_AniObject->m_RootNode->m_Name != "$dummy_root")
			{
				Node* exroot = m_AniObject->m_RootNode;
				m_AniObject->m_RootNode = new Node(NULL);
				m_AniObject->m_RootNode->m_Name = "$dummy_root";
				m_AniObject->m_RootNode->m_Children.push_back(exroot);
				exroot->m_Parent = m_AniObject->m_RootNode;
			}
			// put the new node as its child instead
			m_AniObject->m_RootNode->m_Children.push_back(node);
			node->m_Parent = m_AniObject->m_RootNode;
		}
		else
		{
			// it's the first node imported. place it as root
			m_AniObject->m_RootNode = node;
		}
	}

	// Now inside a frame.
	// read tokens until closing brace is reached.
	bool running = true;
	while (running)
	{
		string objectName = GetNextToken();
		if (objectName.size() == 0)
		{
			ThrowException("Unexpected end of file reached while parsing frame");
		}

		if (objectName == "}")
		{
			break; // frame finished
		}
		else if (objectName == "Frame")
		{
			ParseDataObjectFrame(node); // child frame
		}
		else if (objectName == "FrameTransformMatrix")
		{
			ParseDataObjectTransformationMatrix(node->m_TrafoMatrix);
		}
		else if (objectName == "Mesh")
		{
			Mesh* mesh = new Mesh;
			node->m_Meshes.push_back(mesh);
			m_AniObject->m_GlobalMeshes.push_back(unique_ptr<Mesh>(mesh));
			ParseDataObjectMesh(mesh);
		}
		else
		{
			OutputDebugStringA("Unknown data object in frame in x file");
			ParseUnknownDataObject();
		}
	}
}

void XFileParser::ParseDataObjectTransformationMatrix(CGH::MAT16& pMatrix)
{
	// read header, we're not interested if it has a name
	readHeadOfDataObject();

	// read its components
	pMatrix.m[0][0] = ReadFloat(); pMatrix.m[1][0] = ReadFloat();
	pMatrix.m[2][0] = ReadFloat(); pMatrix.m[3][0] = ReadFloat();
	pMatrix.m[0][1] = ReadFloat(); pMatrix.m[1][1] = ReadFloat();
	pMatrix.m[2][1] = ReadFloat(); pMatrix.m[3][1] = ReadFloat();
	pMatrix.m[0][2] = ReadFloat(); pMatrix.m[1][2] = ReadFloat();
	pMatrix.m[2][2] = ReadFloat(); pMatrix.m[3][2] = ReadFloat();
	pMatrix.m[0][3] = ReadFloat(); pMatrix.m[1][3] = ReadFloat();
	pMatrix.m[2][3] = ReadFloat(); pMatrix.m[3][3] = ReadFloat();

	// trailing symbols
	CheckForSemicolon();
	CheckForClosingBrace();
}

void XFileParser::ParseDataObjectMesh(Ani::Mesh* pMesh)
{
	string name;
	readHeadOfDataObject(&name);

	// read vertex count
	unsigned int numVertices = ReadInt();
	pMesh->m_Positions.resize(numVertices);

	// read vertices
	for (unsigned int a = 0; a < numVertices; a++)
		pMesh->m_Positions[a] = ReadVector3();

	// read position faces
	unsigned int numPosFaces = ReadInt();
	pMesh->m_PosFaces.resize(numPosFaces);
	for (unsigned int a = 0; a < numPosFaces; a++)
	{
		unsigned int numIndices = ReadInt();
		if (numIndices < 3)
		{
			ThrowException("Invalid index count " + to_string(numIndices) + " for face" + to_string(a));
		}

		// read indices
		Face& face = pMesh->m_PosFaces[a];
		for (unsigned int b = 0; b < numIndices; b++)
			face.m_Indices.push_back(ReadInt());
		CheckForSeparator();
	}

	// here, other data objects may follow
	bool running = true;
	while (running)
	{
		string objectName = GetNextToken();

		if (objectName.size() == 0)
		{
			ThrowException("Unexpected end of file while parsing mesh structure");
		}
		else if (objectName == "}")
		{
			break; // mesh finished
		}
		else if (objectName == "MeshNormals")
		{
			ParseDataObjectMeshNormals(pMesh);
		}
		else if (objectName == "MeshTextureCoords")
		{
			ParseDataObjectMeshTextureCoords(pMesh);
		}
		else if (objectName == "MeshVertexColors")
		{
			ParseDataObjectMeshVertexColors(pMesh);
		}
		else if (objectName == "MeshMaterialList")
		{
			ParseDataObjectMeshMaterialList(pMesh);
		}
		else if (objectName == "VertexDuplicationIndices")
		{
			ParseUnknownDataObject(); // we'll ignore vertex duplication indices
		}
		else if (objectName == "XSkinMeshHeader")
		{
			ParseDataObjectSkinMeshHeader(pMesh);
		}
		else if (objectName == "SkinWeights")
		{
			ParseDataObjectSkinWeights(pMesh);
		}
		else
		{
			OutputDebugStringA("Unknown data object in mesh in x file");
			ParseUnknownDataObject();
		}
	}
}

void XFileParser::ParseDataObjectSkinWeights(Ani::Mesh* pMesh)
{
	readHeadOfDataObject();

	string transformNodeName;
	GetNextTokenAsString(transformNodeName);

	pMesh->m_Bones.push_back(Bone());
	Bone& bone = pMesh->m_Bones.back();
	bone.m_Name = transformNodeName;

	// read vertex weights
	unsigned int numWeights = ReadInt();
	bone.m_Weights.reserve(numWeights);

	for (unsigned int a = 0; a < numWeights; a++)
	{
		BoneWeight weight;
		weight.m_Vertex = ReadInt();
		bone.m_Weights.push_back(weight);
	}

	// read vertex weights
	for (unsigned int a = 0; a < numWeights; a++)
	{
		bone.m_Weights[a].m_Weight = ReadFloat();
	}

	// read matrix offset
	bone.m_OffsetMatrix.m[0][0] = ReadFloat(); bone.m_OffsetMatrix.m[1][0] = ReadFloat();
	bone.m_OffsetMatrix.m[2][0] = ReadFloat(); bone.m_OffsetMatrix.m[3][0] = ReadFloat();

	bone.m_OffsetMatrix.m[0][1] = ReadFloat(); bone.m_OffsetMatrix.m[1][1] = ReadFloat();
	bone.m_OffsetMatrix.m[2][1] = ReadFloat(); bone.m_OffsetMatrix.m[3][1] = ReadFloat();

	bone.m_OffsetMatrix.m[0][2] = ReadFloat(); bone.m_OffsetMatrix.m[1][2] = ReadFloat();
	bone.m_OffsetMatrix.m[2][2] = ReadFloat(); bone.m_OffsetMatrix.m[3][2] = ReadFloat();

	bone.m_OffsetMatrix.m[0][3] = ReadFloat(); bone.m_OffsetMatrix.m[1][3] = ReadFloat();
	bone.m_OffsetMatrix.m[2][3] = ReadFloat(); bone.m_OffsetMatrix.m[3][3] = ReadFloat();

	CheckForSemicolon();
	CheckForClosingBrace();
}

void XFileParser::ParseDataObjectSkinMeshHeader(Ani::Mesh* pMesh)
{
	readHeadOfDataObject();

	/*unsigned int maxSkinWeightsPerVertex =*/ ReadInt();
	/*unsigned int maxSkinWeightsPerFace =*/ ReadInt();
	/*unsigned int numBonesInMesh = */ReadInt();

	CheckForClosingBrace();
}

void XFileParser::ParseDataObjectMeshNormals(Ani::Mesh* pMesh)
{
	readHeadOfDataObject();

	// read count
	unsigned int numNormals = ReadInt();
	pMesh->m_Normals.resize(numNormals);

	// read normal vectors
	for (unsigned int a = 0; a < numNormals; a++)
		pMesh->m_Normals[a] = ReadVector3();

	// read normal indices
	unsigned int numFaces = ReadInt();
	if (numFaces != pMesh->m_PosFaces.size())
		ThrowException("Normal face count does not match vertex face count.");

	for (unsigned int a = 0; a < numFaces; a++)
	{
		unsigned int numIndices = ReadInt();
		pMesh->m_NormFaces.push_back(Face());
		Face& face = pMesh->m_NormFaces.back();

		for (unsigned int b = 0; b < numIndices; b++)
			face.m_Indices.push_back(ReadInt());

		CheckForSeparator();
	}

	CheckForClosingBrace();
}

void XFileParser::ParseDataObjectMeshTextureCoords(Ani::Mesh* pMesh)
{
	readHeadOfDataObject();
	if (pMesh->m_NumTextures + 1 > AI_MAX_NUMBER_OF_TEXTURECOORDS)
		ThrowException("Too many sets of texture coordinates");

	vector<XMFLOAT2>& coords = pMesh->m_TexCoords[pMesh->m_NumTextures++];

	unsigned int numCoords = ReadInt();
	if (numCoords != pMesh->m_Positions.size())
		ThrowException("Texture coord count does not match vertex count");

	coords.resize(numCoords);
	for (unsigned int a = 0; a < numCoords; a++)
		coords[a] = ReadVector2();

	CheckForClosingBrace();
}

void XFileParser::ParseDataObjectMeshVertexColors(Ani::Mesh* pMesh)
{
	readHeadOfDataObject();
	if (pMesh->m_NumColorSets + 1 > AI_MAX_NUMBER_OF_COLOR_SETS)
		ThrowException("Too many colorsets");
	vector<XMFLOAT4>& colors = pMesh->m_Colors[pMesh->m_NumColorSets++];

	unsigned int numColors = ReadInt();
	if (numColors != pMesh->m_Positions.size())
		ThrowException("Vertex color count does not match vertex count");

	colors.resize(numColors, XMFLOAT4(0, 0, 0, 1));
	for (unsigned int a = 0; a < numColors; a++)
	{
		unsigned int index = ReadInt();
		if (index >= pMesh->m_Positions.size())
			ThrowException("Vertex color index out of bounds");

		colors[index] = ReadRGBA();
		// HACK: (thom) Maxon Cinema XPort plugin puts a third separator here, kwxPort puts a comma.
		// Ignore gracefully.
		if (!m_IsBinaryFormat)
		{
			FindNextNoneWhiteSpace();
			if (*P == ';' || *P == ',')
				P++;
		}
	}

	CheckForClosingBrace();
}

void XFileParser::ParseDataObjectMeshMaterialList(Ani::Mesh* pMesh)
{
	readHeadOfDataObject();

	// read material count
	/*unsigned int numMaterials =*/ ReadInt();
	// read non triangulated face material index count
	unsigned int numMatIndices = ReadInt();

	// some models have a material index count of 1... to be able to read them we
	// replicate this single material index on every face
	if (numMatIndices != pMesh->m_PosFaces.size() && numMatIndices != 1)
		ThrowException("Per-Face material index count does not match face count.");

	// read per-face material indices
	for (unsigned int a = 0; a < numMatIndices; a++)
		pMesh->m_FaceMaterials.push_back(ReadInt());

	// in version 03.02, the face indices end with two semicolons.
	// commented out version check, as version 03.03 exported from blender also has 2 semicolons
	if (!m_IsBinaryFormat) // && MajorVersion == 3 && MinorVersion <= 2)
	{
		if (P < End && *P == ';')
			++P;
	}

	// if there was only a single material index, replicate it on all faces
	while (pMesh->m_FaceMaterials.size() < pMesh->m_PosFaces.size())
		pMesh->m_FaceMaterials.push_back(pMesh->m_FaceMaterials.front());

	// read following data objects
	bool running = true;
	while (running)
	{
		string objectName = GetNextToken();
		if (objectName.size() == 0)
		{
			ThrowException("Unexpected end of file while parsing mesh material list.");
		}
		else if (objectName == "}")
		{
			break; // material list finished
		}
		else if (objectName == "{")
		{
			// template materials 
			string matName = GetNextToken();
			Material material;
			material.m_IsReference = true;
			material.m_Name = matName;
			pMesh->m_Materials.push_back(material);

			CheckForClosingBrace(); // skip }
		}
		else if (objectName == "Material")
		{
			pMesh->m_Materials.push_back(Material());
			ParseDataObjectMaterial(&pMesh->m_Materials.back());
		}
		else if (objectName == ";")
		{
			// ignore
		}
		else
		{
			OutputDebugStringA("Unknown data object in material list in x file");
			ParseUnknownDataObject();
		}
	}
}

void XFileParser::ParseDataObjectMaterial(Ani::Material* pMaterial)
{
	string matName;
	readHeadOfDataObject(&matName);
	if (matName.empty())
	{
		matName = string("material") + to_string(m_LineNumber);
	}

	pMaterial->m_Name = matName;
	pMaterial->m_IsReference = false;

	// read material values
	pMaterial->m_Diffuse = ReadRGBA();
	pMaterial->m_SpecularExponent = ReadFloat();
	pMaterial->m_Specular = ReadRGB();
	pMaterial->m_Emissive = ReadRGB();

	// read other data objects
	bool running = true;
	while (running)
	{
		string objectName = GetNextToken();
		if (objectName.size() == 0)
		{
			ThrowException("Unexpected end of file while parsing mesh material");
		}
		else if (objectName == "}")
		{
			break; // material finished
		}
		else if (objectName == "TextureFilename" || objectName == "TextureFileName")
		{
			// some exporters write "TextureFileName" instead.
			string texname;
			ParseDataObjectTextureFilename(texname);
			pMaterial->m_Textures.push_back(TexEntry(texname));
		}
		else if (objectName == "NormalmapFilename" || objectName == "NormalmapFileName")
		{
			// one exporter writes out the normal map in a separate filename tag
			string texname;
			ParseDataObjectTextureFilename(texname);
			pMaterial->m_Textures.push_back(TexEntry(texname, true));
		}
		else
		{
			OutputDebugStringA("Unknown data object in material in x file");
			ParseUnknownDataObject();
		}
	}
}

void XFileParser::ParseDataObjectAnimTicksPerSecond()
{
	readHeadOfDataObject();
	m_AniObject->m_AnimTicksPerSecond = ReadInt();
	CheckForClosingBrace();
}

void XFileParser::ParseDataObjectAnimationSet()
{
	string animName;
	readHeadOfDataObject(&animName);

	m_AniObject->m_Anims.push_back(Animation());
	m_AniObject->m_Anims.back().m_Name = animName;

	bool running = true;
	while (running)
	{
		string objectName = GetNextToken();
		if (objectName.length() == 0)
		{
			ThrowException("Unexpected end of file while parsing animation set.");
		}
		else if (objectName == "}")
		{
			break; // animation set finished
		}
		else if (objectName == "Animation")
		{
			ParseDataObjectAnimation(&m_AniObject->m_Anims.back());
		}
		else
		{
			OutputDebugStringA("Unknown data object in animation set in x file");
			ParseUnknownDataObject();
		}
	}
}

void XFileParser::ParseDataObjectAnimation(Ani::Animation* pAnim)
{
	readHeadOfDataObject();
	pAnim->m_Anims.push_back(AnimBone());
	AnimBone* banim = &pAnim->m_Anims.back();

	bool running = true;
	while (running)
	{
		string objectName = GetNextToken();

		if (objectName.length() == 0)
		{
			ThrowException("Unexpected end of file while parsing animation.");
		}
		else if (objectName == "}")
		{
			break; // animation finished
		}
		else if (objectName == "AnimationKey")
		{
			ParseDataObjectAnimationKey(banim);
		}
		else if (objectName == "AnimationOptions")
		{
			ParseUnknownDataObject(); // not interested
		}
		else if (objectName == "{")
		{
			// read frame name
			banim->m_BoneName = GetNextToken();
			CheckForClosingBrace();
		}
		else
		{
			OutputDebugStringA("Unknown data object in animation in x file");
			ParseUnknownDataObject();
		}
	}
}

void XFileParser::ParseDataObjectAnimationKey(Ani::AnimBone* pAnimBone)
{
	readHeadOfDataObject();

	// read key type
	unsigned int keyType = ReadInt();

	// read number of keys
	unsigned int numKeys = ReadInt();

	for (unsigned int a = 0; a < numKeys; a++)
	{
		// read time
		unsigned int time = ReadInt();

		// read keys
		switch (keyType)
		{
		case 0: // rotation quaternion
		{
			// read count
			int readCount = ReadInt();
			if (readCount != 4)
			{
				ThrowException("Invalid number of arguments for quaternion key in animation");
			}

			TimeValue<XMFLOAT4> key;
			key.m_Time = double(time);
			key.m_Value.w = ReadFloat();
			key.m_Value.x = ReadFloat();
			key.m_Value.y = ReadFloat();
			key.m_Value.z = ReadFloat();
			pAnimBone->m_RotKeys.push_back(key);

			CheckForSemicolon();
			break;
		}
		case 1: // scale vector
		case 2: // position vector
		{
			// read count
			if (ReadInt() != 3)
				ThrowException("Invalid number of arguments for vector key in animation");

			TimeValue<XMFLOAT3> key;
			key.m_Time = double(time);
			key.m_Value = ReadVector3();

			if (keyType == 2)
				pAnimBone->m_PosKeys.push_back(key);
			else
				pAnimBone->m_ScaleKeys.push_back(key);

			CheckForSemicolon();
			break;
		}
		case 3: // combined transformation matrix
		case 4: // denoted both as 3 or as 4
		{
			// read count
			if (ReadInt() != 16)
				ThrowException("Invalid number of arguments for matrix key in animation");

			// read matrix
			TimeValue<CGH::MAT16> key;
			key.m_Time = double(time);
			key.m_Value.m[0][0] = ReadFloat(); key.m_Value.m[1][0] = ReadFloat();
			key.m_Value.m[2][0] = ReadFloat(); key.m_Value.m[3][0] = ReadFloat();
			key.m_Value.m[0][1] = ReadFloat(); key.m_Value.m[1][1] = ReadFloat();
			key.m_Value.m[2][1] = ReadFloat(); key.m_Value.m[3][1] = ReadFloat();
			key.m_Value.m[0][2] = ReadFloat(); key.m_Value.m[1][2] = ReadFloat();
			key.m_Value.m[2][2] = ReadFloat(); key.m_Value.m[3][2] = ReadFloat();
			key.m_Value.m[0][3] = ReadFloat(); key.m_Value.m[1][3] = ReadFloat();
			key.m_Value.m[2][3] = ReadFloat(); key.m_Value.m[3][3] = ReadFloat();
			pAnimBone->m_TrafoKeys.push_back(key);

			CheckForSemicolon();
			break;
		}

		default:
			ThrowException("Unknown key type " + to_string(keyType) + " in animation.");
			break;
		} // end switch

		// key separator
		CheckForSeparator();
	}

	CheckForClosingBrace();
}

void XFileParser::ParseDataObjectTextureFilename(string& pName)
{
	readHeadOfDataObject();
	GetNextTokenAsString(pName);
	CheckForClosingBrace();

	// FIX: some files (e.g. AnimationTest.x) have "" as texture file name
	if (!pName.length())
	{
		OutputDebugStringA("Length of texture file name is zero. Skipping this texture.");
	}

	// some exporters write double backslash paths out. We simply replace them if we find them
	while (pName.find("\\\\") != string::npos)
	{
		pName.replace(pName.find("\\\\"), 2, "\\");
	}
}

void XFileParser::ParseUnknownDataObject()
{
	// find opening delimiter
	bool running = true;
	while (running)
	{
		string t = GetNextToken();
		if (t.length() == 0)
		{
			ThrowException("Unexpected end of file while parsing unknown segment.");
		}

		if (t == "{")
		{
			break;
		}
	}

	unsigned int counter = 1;

	// parse until closing delimiter
	while (counter > 0)
	{
		string t = GetNextToken();

		if (t.length() == 0)
		{
			ThrowException("Unexpected end of file while parsing unknown segment.");
		}

		if (t == "{")
		{
			++counter;
		}
		else if (t == "}")
		{
			--counter;
		}
	}
}

void XFileParser::FindNextNoneWhiteSpace()
{
	if (m_IsBinaryFormat)
		return;

	bool running = true;
	while (running)
	{
		while (P < End && isspace((unsigned char)* P))
		{
			if (*P == '\n')
			{
				m_LineNumber++;
			}

			++P;
		}

		if (P >= End) return;

		// check if this is a comment
		if ((P[0] == '/' && P[1] == '/') || P[0] == '#')
		{
			ReadUntilEndOfLine();
		}
		else
		{
			break;
		}
	}
}

string XFileParser::GetNextToken()
{
	string s;

	// process binary-formatted file
	if (m_IsBinaryFormat)
	{
		// in binary mode it will only return NAME and STRING token
		// and (correctly) skip over other tokens.

		if (End - P < 2) return s;
		unsigned int tok = ReadBinWord();
		unsigned int len;

		// standalone tokens
		switch (tok)
		{
		case 1:
			// name token
			if (End - P < 4) return s;
			len = ReadBinDWord();
			if (End - P < int(len)) return s;
			s = string(P, len);
			P += len;
			return s;
		case 2:
			// string token
			if (End - P < 4) return s;
			len = ReadBinDWord();
			if (End - P < int(len)) return s;
			s = string(P, len);
			P += (len + 2);
			return s;
		case 3:
			// integer token
			P += 4;
			return "<integer>";
		case 5:
			// GUID token
			P += 16;
			return "<guid>";
		case 6:
			if (End - P < 4) return s;
			len = ReadBinDWord();
			P += (len * 4);
			return "<int_list>";
		case 7:
			if (End - P < 4) return s;
			len = ReadBinDWord();
			P += (len * m_BinaryFloatSize);
			return "<flt_list>";
		case 0x0a:
			return "{";
		case 0x0b:
			return "}";
		case 0x0c:
			return "(";
		case 0x0d:
			return ")";
		case 0x0e:
			return "[";
		case 0x0f:
			return "]";
		case 0x10:
			return "<";
		case 0x11:
			return ">";
		case 0x12:
			return ".";
		case 0x13:
			return ",";
		case 0x14:
			return ";";
		case 0x1f:
			return "template";
		case 0x28:
			return "WORD";
		case 0x29:
			return "DWORD";
		case 0x2a:
			return "FLOAT";
		case 0x2b:
			return "DOUBLE";
		case 0x2c:
			return "CHAR";
		case 0x2d:
			return "UCHAR";
		case 0x2e:
			return "SWORD";
		case 0x2f:
			return "SDWORD";
		case 0x30:
			return "void";
		case 0x31:
			return "string";
		case 0x32:
			return "unicode";
		case 0x33:
			return "cstring";
		case 0x34:
			return "array";
		}
	}
	// process text-formatted file
	else
	{
		FindNextNoneWhiteSpace();
		if (P >= End)
		{
			return s;
		}

		while ((P < End) && !isspace((unsigned char)* P))
		{
			// either keep token delimiters when already holding a token, or return if first valid char
			if (*P == ';' || *P == '}' || *P == '{' || *P == ',')
			{
				if (!s.size())
				{
					s.append(P++, 1);
				}

				break; // stop for delimiter
			}
			s.append(P++, 1);
		}
	}
	return s;
}

void XFileParser::readHeadOfDataObject(string* poName)
{
	string nameOrBrace = GetNextToken();
	if (nameOrBrace != "{")
	{
		if (poName)
		{
			*poName = nameOrBrace;
		}

		if (GetNextToken() != "{")
		{
			ThrowException("Opening brace expected.");
		}
	}
}

void XFileParser::CheckForClosingBrace()
{
	if (GetNextToken() != "}")
	{
		ThrowException("Closing brace expected.");
	}
}

void XFileParser::CheckForSemicolon()
{
	if (m_IsBinaryFormat) return;

	if (GetNextToken() != ";")
	{
		ThrowException("Semicolon expected.");
	}
}

void XFileParser::CheckForSeparator()
{
	if (m_IsBinaryFormat) return;

	string token = GetNextToken();
	if (token != "," && token != ";")
	{
		ThrowException("Separator character (';' or ',') expected.");
	}
}

void XFileParser::TestForSeparator()
{
	if (m_IsBinaryFormat) return;

	FindNextNoneWhiteSpace();
	if (P >= End) return;

	// test and skip
	if (*P == ';' || *P == ',')
	{
		P++;
	}
}

void XFileParser::GetNextTokenAsString(string& poString)
{
	if (m_IsBinaryFormat)
	{
		poString = GetNextToken();
		return;
	}

	FindNextNoneWhiteSpace();
	if (P >= End)
	{
		ThrowException("Unexpected end of file while parsing string");
	}

	if (*P != '"')
	{
		ThrowException("Expected quotation mark.");
	}

	++P;

	while (P < End && *P != '"')
	{
		poString.append(P++, 1);
	}

	if (P >= End - 1)
	{
		ThrowException("Unexpected end of file while parsing string");
	}

	if (P[1] != ';' || P[0] != '"')
	{
		ThrowException("Expected quotation mark and semicolon at the end of a string.");
	}

	P += 2;
}

void XFileParser::ReadUntilEndOfLine()
{
	if (m_IsBinaryFormat) return;

	while (P < End)
	{
		if (*P == '\n' || *P == '\r')
		{
			++P; m_LineNumber++;
			return;
		}

		++P;
	}
}

unsigned short XFileParser::ReadBinWord()
{
	assert(End - P >= 2);
	const unsigned char* q = (const unsigned char*)P;
	unsigned short tmp = q[0] | (q[1] << 8);
	P += 2;

	return tmp;
}

unsigned int XFileParser::ReadBinDWord()
{
	assert(End - P >= 4);
	const unsigned char* q = (const unsigned char*)P;
	unsigned int tmp = q[0] | (q[1] << 8) | (q[2] << 16) | (q[3] << 24);
	P += 4;

	return tmp;
}

unsigned int XFileParser::ReadInt()
{
	if (m_IsBinaryFormat)
	{
		if (m_BinaryNumCount == 0 && End - P >= 2)
		{
			unsigned short tmp = ReadBinWord(); // 0x06 or 0x03
			if (tmp == 0x06 && End - P >= 4) // array of ints follows
				m_BinaryNumCount = ReadBinDWord();
			else // single int follows
				m_BinaryNumCount = 1;
		}

		--m_BinaryNumCount;
		if (End - P >= 4) {
			return ReadBinDWord();
		}
		else {
			P = End;
			return 0;
		}
	}
	else
	{
		FindNextNoneWhiteSpace();

		// TODO: consider using strtol10 instead???

		// check preceeding minus sign
		bool isNegative = false;
		if (*P == '-')
		{
			isNegative = true;
			P++;
		}

		// at least one digit expected
		if (!isdigit(*P))
			ThrowException("Number expected.");

		// read digits
		unsigned int number = 0;
		while (P < End)
		{
			if (!isdigit(*P))
				break;
			number = number * 10 + (*P - 48);
			P++;
		}

		CheckForSeparator();
		return isNegative ? ((unsigned int)-int(number)) : number;
	}
}

float XFileParser::ReadFloat()
{
	if (m_IsBinaryFormat)
	{
		if (m_BinaryNumCount == 0 && End - P >= 2)
		{
			unsigned short tmp = ReadBinWord(); // 0x07 or 0x42
			if (tmp == 0x07 && End - P >= 4) // array of floats following
			{
				m_BinaryNumCount = ReadBinDWord();
			}
			else // single float following
			{
				m_BinaryNumCount = 1;
			}
		}

		--m_BinaryNumCount;
		if (m_BinaryFloatSize == 8)
		{
			if (End - P >= 8)
			{
				float result = (float)(*(double*)P);
				P += 8;
				return result;
			}
			else
			{
				P = End;
				return 0;
			}
		}
		else
		{
			if (End - P >= 4)
			{
				float result = *(float*)P;
				P += 4;
				return result;
			}
			else
			{
				P = End;
				return 0;
			}
		}
	}

	// text version
	FindNextNoneWhiteSpace();
	// check for various special strings to allow reading files from faulty exporters
	// I mean you, Blender!
	// Reading is safe because of the terminating zero
	if (strncmp(P, "-1.#IND00", 9) == 0 || strncmp(P, "1.#IND00", 8) == 0)
	{
		P += 9;
		CheckForSeparator();
		return 0.0f;
	}
	else if (strncmp(P, "1.#QNAN0", 8) == 0)
	{
		P += 8;
		CheckForSeparator();
		return 0.0f;
	}

	float result = 0.0f;
	P = Assimp::fast_atoreal_move<float>(P, result);

	CheckForSeparator();

	return result;
}

DirectX::XMFLOAT2 XFileParser::ReadVector2()
{
	XMFLOAT2 vector;
	vector.x = ReadFloat();
	vector.y = ReadFloat();
	TestForSeparator();

	return vector;
}

DirectX::XMFLOAT3 XFileParser::ReadVector3()
{
	XMFLOAT3 vector;
	vector.x = ReadFloat();
	vector.y = ReadFloat();
	vector.z = ReadFloat();
	TestForSeparator();

	return vector;
}

DirectX::XMFLOAT3 XFileParser::ReadRGB()
{
	XMFLOAT3 color;
	color.x = ReadFloat();
	color.y = ReadFloat();
	color.z = ReadFloat();
	TestForSeparator();

	return color;
}

DirectX::XMFLOAT4 XFileParser::ReadRGBA()
{
	XMFLOAT4 color;
	color.x = ReadFloat();
	color.y = ReadFloat();
	color.z = ReadFloat();
	color.w = ReadFloat();
	TestForSeparator();

	return color;
}

void XFileParser::ThrowException(const string& pText)
{
	if (m_IsBinaryFormat)
	{
		throw exception(pText.c_str());
	}
	else
	{
		throw exception(("Line " + to_string(m_LineNumber) + " : " + pText).c_str());
	}
}

void XFileParser::FilterHierarchy(Ani::Node* pNode)
{
	// if the node has just a single unnamed child containing a mesh, remove
	// the anonymous node inbetween. The 3DSMax kwXport plugin seems to produce this
	// mess in some cases
	if (pNode->m_Children.size() == 1 && pNode->m_Meshes.empty())
	{
		Node* child = pNode->m_Children.front();
		if (child->m_Name.length() == 0 && child->m_Meshes.size() > 0)
		{
			// transfer its meshes to us
			for (unsigned int a = 0; a < child->m_Meshes.size(); a++)
			{
				pNode->m_Meshes.push_back(child->m_Meshes[a]);
			}

			child->m_Meshes.clear();

			// transfer the transform as well
			pNode->m_TrafoMatrix = pNode->m_TrafoMatrix * child->m_TrafoMatrix;

			// then kill it
			delete child;
			pNode->m_Children.clear();
		}
	}

	// recurse
	for (unsigned int a = 0; a < pNode->m_Children.size(); a++)
	{
		FilterHierarchy(pNode->m_Children[a]);
	}
}


