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

void HeightMap::SetScale(const physx::PxVec3 scale)
{
	m_Scale = scale;

	PxHeightFieldGeometry fieldGeo;

	if (m_Shape->getHeightFieldGeometry(fieldGeo))
	{
		fieldGeo.heightScale = m_Scale.y;
		fieldGeo.columnScale = m_Scale.z;
		fieldGeo.rowScale = m_Scale.x;

		auto transform = GetComponent<DOTransform>();
		transform->SetScale(m_Scale);

		m_Shape->setGeometry(fieldGeo);
	}

	auto mesh = GetComponent<DORenderMesh>();

	if (mesh)
	{
		mesh->ReComputeHeightField(m_Scale);
	}
}

void HeightMap::AddMapPickingWrok(std::function<void(const physx::PxVec3 & pickingPos)> func)
{
	m_MapPickingWorks.push_back(func);
}

void HeightMap::ClearMapPickingWork()
{
	m_MapPickingWorks.clear();
}

void HeightMap::Init(IGraphicDevice* graphicDevice, ISoundDevice* soundDevice, PhysX4_1* physxDevice)
{
	std::wstring extension;
	m_fileName = GetFileNameFromPath(m_filePath, extension);

	assert(extension == L"raw");

	if (m_GetPickingPosFunc == nullptr)
	{
		m_GetPickingPosFunc = std::bind(&PhysX4_1::GetPickingPos, physxDevice);
	}

	unsigned int fileSizeHight = 0;
	unsigned int fileSizeLow = 0;
	std::vector<int> datas;
	std::vector<float> heights;
	LoadRAWFile(m_filePath, fileSizeHight, fileSizeLow, datas);
	CreateRigidStatic(physxDevice, fileSizeHight, fileSizeLow, datas, heights);
	CreateRenderMesh(graphicDevice, fileSizeHight, fileSizeLow, heights);
}

void HeightMap::LoadRAWFile(const std::wstring& filePath, unsigned int& fileHeight, unsigned int& fileWidth, std::vector<int>& datas)
{
	_WIN32_FILE_ATTRIBUTE_DATA fileInfo = {};
	GetFileAttributesEx(m_filePath.c_str(), GET_FILEEX_INFO_LEVELS::GetFileExInfoStandard, reinterpret_cast<LPVOID>(&fileInfo));

	if (fileInfo.nFileSizeHigh == 0)
	{
		fileWidth = static_cast<int>(sqrt(long double(fileInfo.nFileSizeLow)));
		fileHeight = fileWidth;
	}
	else
	{
		fileWidth = fileInfo.nFileSizeLow;
		fileHeight = fileInfo.nFileSizeHigh;
	}

	m_MapOriginSize.x = static_cast<float>(fileWidth);
	m_MapOriginSize.y = static_cast<float>(fileHeight);
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

void HeightMap::CreateRigidStatic(PhysX4_1* pxd, unsigned int fileHeight, unsigned int fileWidth, const std::vector<int>& datas, std::vector<float>& heights)
{
	auto pxDevice = pxd->GetPhysics();
	auto cooking = pxd->GetCooking();
	const int samplesSize = CGH::SizeTTransINT(datas.size());

	std::vector<PxHeightFieldSample> samples(samplesSize);
	heights.resize(samplesSize);

	unsigned int index = 0;
	for (int y = fileHeight-1; y >= 0; y--)
	{
	 	unsigned int baseIndex = y * fileWidth;
		for (unsigned int x = 0; x < fileWidth; x++)
		{
			samples[index].height = datas[static_cast<size_t>(baseIndex)+x];
			samples[index].materialIndex0 = 0;
			samples[index].materialIndex1 = 0;
			samples[index].setTessFlag();
			index++;
		}
	}

	PxHeightFieldDesc fieldDesc = {};
	fieldDesc.format = PxHeightFieldFormat::eS16_TM;
	fieldDesc.nbRows = fileHeight;
	fieldDesc.nbColumns = fileWidth;
	fieldDesc.samples.data = samples.data();
	fieldDesc.samples.stride = sizeof(PxHeightFieldSample);
	auto field = cooking->createHeightField(fieldDesc, pxDevice->getPhysicsInsertionCallback());

	field->saveCells(samples.data(), samplesSize * sizeof(PxHeightFieldSample));

	index = 0;
	for (int y = fileHeight - 1; y >= 0; y--)
	{
		unsigned int baseIndex = y * fileWidth;
		for (unsigned int x = 0; x < fileWidth; x++)
		{
			heights[index] = samples[static_cast<size_t>(baseIndex) + x].height;
			index++;
		}
	}

	PxHeightFieldGeometry fieldGeo(field, PxMeshGeometryFlags(), m_Scale.y, m_Scale.x, m_Scale.z);

	m_Shape = pxDevice->createShape(fieldGeo, *pxd->GetBaseMaterial(), true);

	m_PxStatic = PxCreateStatic(*pxDevice, PxTransform(PxIdentity), *m_Shape);
	pxd->GetScene(m_Scene)->addActor(*m_PxStatic);

	m_Funcs = std::make_unique<PhysXFunctionalObject>(this);
	m_Funcs->m_VoidFuncs.push_back(std::bind(&HeightMap::StartMapPickingWork, this));

	m_PxStatic->userData = m_Funcs.get();
}

void HeightMap::CreateRenderMesh(IGraphicDevice* gd, unsigned int fileHeight, unsigned int fileWidth, const std::vector<float>& heights)
{
	const size_t numVertices = heights.size();
	std::vector<unsigned int> indices;

	Material testMaterial;
	testMaterial.diffuseAlbedo = { 1,1,1,1 };
	gd->CreateMaterials({ "HeightFieldMaterial" }, { testMaterial });

	MeshObject meshObject;
	SubmeshData oneSub;
	oneSub.indexOffset = 0;
	oneSub.vertexOffset = 0;
	oneSub.numIndex = CGH::SizeTTransUINT(indices.size());
	oneSub.numVertex = CGH::SizeTTransUINT(numVertices);
	oneSub.material = "HeightFieldMaterial";
	oneSub.diffuseMap = "HeightMap3.jpg";

	meshObject.subs["oneSub"] = oneSub;

	std::string meshName(m_fileName.begin(), m_fileName.end());

	auto transfrom = CreateComponenet<DOTransform>();
	auto renderer = CreateComponenet<DORenderer>();
	auto mesh = CreateComponenet<DORenderMesh>();

	gd->CreateMesh(meshName, meshObject, CGH::HEIGHTFIELD_MESH, CGH::SizeTTransUINT(numVertices), heights.data(), indices);
	mesh->SelectMesh(CGH::HEIGHTFIELD_MESH, meshName);
	mesh->ReComputeHeightField(m_Scale);

	renderer->GetRenderInfo().type = RENDER_HEIGHT_FIELD;
}

void HeightMap::StartMapPickingWork()
{
	PxVec3 pickingPos = m_GetPickingPosFunc();

	for (auto& it : m_MapPickingWorks)
	{
		it(pickingPos);
	}
}