#include "HeightMap.h"
#include <Windows.h>
#include "PhysicsDO.h"
#include "GraphicDO.h"
#include "PhysX4_1.h"
#include "IGraphicDevice.h"
#include "d3dUtil.h"

std::function<const physx::PxVec3 & (void)> HeightMap::m_GetPickingPosFunc = nullptr;

using namespace physx;

void HeightMap::Delete()
{
	if (m_PxStatic)
	{
		m_PxStatic->getScene()->removeActor(*m_PxStatic);
		m_PxStatic = nullptr;
	}

	DeviceObject::Delete();
}

void HeightMap::AddMapPickingWrok(std::function<void(const physx::PxVec3& pickingPos)> func)
{
	m_MapPickingWorks.push_back(func);
}

void HeightMap::ClearMapPickingWork()
{
	m_MapPickingWorks.clear();
}

void HeightMap::Init(PhysX4_1* pxd, IGraphicDevice* gd)
{
	std::wstring extension;
	m_fileName = GetFileNameFromPath(m_filePath, extension);

	assert(extension == L"raw");

	if (m_GetPickingPosFunc == nullptr)
	{
		m_GetPickingPosFunc = std::bind(&PhysX4_1::GetPickingPos, pxd);
	}

	int fileSizeHight = 0;
	int fileSizeLow = 0;
	std::vector<int> datas;
	std::vector<float> heights;
	LoadRAWFile(m_filePath, fileSizeHight, fileSizeLow, datas);
	CreateRigidStatic(pxd, fileSizeHight, fileSizeLow, datas, heights);
	CreateRenderMesh(gd, fileSizeHight, fileSizeLow, heights);
}

void HeightMap::LoadRAWFile(const std::wstring& filePath, int& fileHeight, int& fileWidth, std::vector<int>& datas)
{
	_WIN32_FILE_ATTRIBUTE_DATA fileInfo = {};
	GetFileAttributesEx(m_filePath.c_str(), GET_FILEEX_INFO_LEVELS::GetFileExInfoStandard, reinterpret_cast<LPVOID>(&fileInfo));
	
	if (fileInfo.nFileSizeHigh == 0)
	{
		fileWidth = sqrt(long double(fileInfo.nFileSizeLow));
		fileHeight = fileWidth;
	}
	else
	{
		fileWidth = fileInfo.nFileSizeLow;
		fileHeight = fileInfo.nFileSizeHigh;
	}

	FILE* fp = nullptr;

	_wfopen_s(&fp, filePath.c_str(), L"rb");

	const int numData = fileHeight * fileWidth;
	int readNum = 0;

	datas.reserve(numData);

	if (fp)
	{
		for (; !feof(fp); readNum++)
		{
			datas.push_back(fgetc(fp));
		}
	}

	if (fp)
	{
		fclose(fp);
	}
	
	datas.pop_back();
	readNum--;
	assert(numData == readNum);
}

void HeightMap::CreateRigidStatic(PhysX4_1* pxd, int fileHeight, int fileWidth, const std::vector<int>& datas, std::vector<float>& heights)
{
	auto pxDevice = pxd->GetPhysics();
	auto cooking = pxd->GetCooking();
	const int samplesSize = datas.size();

	std::vector<PxHeightFieldSample> samples(samplesSize);
	heights.resize(samplesSize);
	
	for (int i = 0; i < samplesSize; i++)
	{
		samples[i].height = datas[i];
		samples[i].materialIndex0 = 0;
		samples[i].materialIndex1 = 0;
	}

	PxHeightFieldDesc fieldDesc = {};
	fieldDesc.format = PxHeightFieldFormat::eS16_TM;
	fieldDesc.nbRows = fileHeight;
	fieldDesc.nbColumns = fileWidth;
	fieldDesc.samples.data = samples.data();
	fieldDesc.samples.stride = sizeof(PxHeightFieldSample);
	auto field = cooking->createHeightField(fieldDesc, pxDevice->getPhysicsInsertionCallback());
	
	field->saveCells(samples.data(), samplesSize * sizeof(PxHeightFieldSample));

	for (int i = 0; i < samplesSize; i++)
	{
		heights[i] = samples[i].height;
	}
	
	PxHeightFieldGeometry fieldGeo(field, PxMeshGeometryFlags(), m_Scale.y, m_Scale.x, m_Scale.z);

	auto shape = pxDevice->createShape(fieldGeo, *pxd->GetBaseMaterial(), true);

	m_PxStatic = PxCreateStatic(*pxDevice, PxTransform(PxIdentity), *shape);
	pxd->GetScene(m_Scene)->addActor(*m_PxStatic);
	shape->release();

	m_Funcs = std::make_unique<PhysXFunctionalObject>(this);
	m_Funcs->m_VoidFuncs.push_back(std::bind(&HeightMap::StartMapPickingWork, this));

	m_PxStatic->userData = m_Funcs.get();
	pxd->GetScene(GetScene())->addActor(*m_PxStatic);
}

void HeightMap::CreateRenderMesh(IGraphicDevice* gd, int fileHeight, int fileWidth, const std::vector<float>& heights)
{
	const size_t numVertices = heights.size();
	std::vector<Vertex> vertices(numVertices);
	std::vector<unsigned int> indices;

	for (size_t i = 0; i < numVertices; i++)
	{
		int indexY = i % fileWidth;
		int indexX = i / fileWidth;

		vertices[i].uv.x = (float)indexY / (fileWidth - 1);
		vertices[i].uv.y = (float)indexX / (fileHeight - 1);

		vertices[i].position.y = heights[i];
		vertices[i].position.z = indexY;
		vertices[i].position.x = indexX;
	}

	for (size_t i = 0; i < numVertices; i++)
	{
		int indexX = i % fileWidth;
		int indexY = i / fileWidth;

		if (indexX < (fileWidth - 1) && indexY < (fileHeight - 1))
		{
			PxVec3 rightVec = vertices[i + 1].position - vertices[i].position;
			PxVec3 upVec = vertices[i + fileWidth].position - vertices[i].position;

			upVec.x *= m_Scale.x;
			upVec.y *= m_Scale.y;
			upVec.z *= m_Scale.z;

			rightVec.x *= m_Scale.x;
			rightVec.y *= m_Scale.y;
			rightVec.z *= m_Scale.z;

			PxVec3 normalVec = -upVec.cross(rightVec);

			vertices[i].normal = normalVec.getNormalized();
		}
		else
		{
			vertices[i].normal = { 0,1,0 };
		}
	}

	for (size_t i = 0; i < numVertices; i++)
	{
		int indexX = i % fileWidth;
		int indexY = i / fileWidth;

		if (indexX < (fileWidth - 1) && indexY < (fileHeight - 1))
		{
			indices.push_back(i + 1);
			indices.push_back(i + fileWidth);
			indices.push_back(i + 0);
			indices.push_back(i + 1);
			indices.push_back(i + fileHeight + 1);
			indices.push_back(i + fileWidth);

			assert(numVertices > i + fileHeight + 1);
		}
	}

	Material testMaterial;
	testMaterial.diffuseMapIndex = gd->GetTextureIndex("HeightMap3.jpg");
	testMaterial.diffuseAlbedo = { 1,1,1,1 };
	assert(gd->CreateMaterials({ "testMaterial" }, { testMaterial }));

	std::string meshName(m_fileName.begin(), m_fileName.end());
	SubmeshData oneSub;
	oneSub.indexOffset = 0;
	oneSub.vertexOffset = 0;
	oneSub.numIndex = indices.size();
	oneSub.numVertex = vertices.size();
	oneSub.material = "testMaterial";

	MeshObject meshOb;
	meshOb.subs["oneSub"] = oneSub;
	assert(gd->CreateMesh(meshName.c_str(), meshOb, vertices, indices));

	auto renderMesh = CreateComponenet<DORenderMesh>();
	auto renderer = CreateComponenet<DORenderer>();
	auto transfrom = CreateComponenet<DOTransform>();
	transfrom->SetScale(m_Scale);

	renderMesh->SelectMesh(meshName);

	RenderInfo testRenderInfo(RENDER_MESH);
	renderer->SetRenderInfo(testRenderInfo);
}

void HeightMap::StartMapPickingWork()
{
	PxVec3 pickingPos = m_GetPickingPosFunc();

	for (auto& it : m_MapPickingWorks)
	{
		it(pickingPos);
	}
}
