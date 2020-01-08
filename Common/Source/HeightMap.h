#pragma once
#include "PhysicsObject.h"
#include <string>
#include <vector>

namespace physx
{
	class PxRigidStatic;
}

class HeightMap :public PhyscisObject
{
public:
	HeightMap(CGHScene& scene, GameObject* parent, const char* typeName, const wchar_t* filePath)
		: PhyscisObject(scene, parent, typeName)
		, m_PxStatic(nullptr)
		, m_filePath(filePath)
	{
		 
	}

	virtual void Delete() override;

private:
	virtual void Update(float delta) override;
	virtual void Init(PhysX4_1* pdx, IGraphicDevice* gd) override;

private:
	void LoadRAWFile(const std::wstring& filePath, int& fileHeight, int& fileWidth, std::vector<int>& datas);
	void CreateRigidStatic(PhysX4_1* pxd, int fileHeight, int fileWidth, const std::vector<int>& datas, std::vector<float>& heights);
	void CreateRenderMesh(IGraphicDevice* gd, int fileHeight, int fileWidth, const std::vector<float>& heights);

private:
	physx::PxRigidStatic*	m_PxStatic;
	std::wstring			m_filePath;
};