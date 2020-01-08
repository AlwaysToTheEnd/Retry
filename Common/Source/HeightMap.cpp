#include "HeightMap.h"
#include <Windows.h>
#include "PhysicsDO.h"
#include "GraphicDO.h"
#include "PhysX4_1.h"
#include "d3dUtil.h"

using namespace physx;

void HeightMap::Delete()
{
	DeviceObject::Delete();
}

void HeightMap::Update(float delta)
{

}

void HeightMap::Init(PhysX4_1* pxd, IGraphicDevice* gd)
{
	std::wstring extension;
	std::wstring fileName = GetFileNameFromPath(m_filePath, extension);

	assert(extension == L"raw");

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
	}

	PxHeightFieldDesc fieldDesc = {};
	fieldDesc.format = PxHeightFieldFormat::eS16_TM;
	fieldDesc.nbRows = fileHeight;
	fieldDesc.nbColumns = fileWidth;
	fieldDesc.samples.data = samples.data();
	fieldDesc.samples.stride = sizeof(PxHeightFieldSample);
	auto field = cooking->createHeightField(fieldDesc, pxDevice->getPhysicsInsertionCallback());

	for (int i = 0; i < fileWidth; i++)
	{
		const int currWidthIndex = i * fileWidth;
		for (int j = 0; j < fileHeight; j++)
		{
			heights[currWidthIndex + j] = field->getSample(i, j).height;
		}
	}
	
	PxHeightFieldGeometry fieldGeo(field, PxMeshGeometryFlags(), 1, 1, 1);

	auto shape = pxDevice->createShape(fieldGeo, *pxd->GetBaseMaterial(), true);

	m_PxStatic = PxCreateStatic(*pxDevice, PxTransform(PxIdentity), *shape);

	shape->release();
}

void HeightMap::CreateRenderMesh(IGraphicDevice* gd, int fileHeight, int fileWidth, const std::vector<float>& heights)
{
	
}
