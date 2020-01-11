#pragma once
#include "PhysicsObject.h"
#include "PhysXFunctionalObject.h"
#include <memory>
#include <functional>
#include <string>
#include <vector>

namespace physx
{
	class PxRigidStatic;
	class PxShape;
}

class HeightMap :public PhyscisObject
{
public:
	HeightMap(CGHScene& scene, GameObject* parent, const char* typeName, const wchar_t* filePath, physx::PxVec3 scale)
		: PhyscisObject(scene, parent, typeName)
		, m_Scale(scale)
		, m_filePath(filePath)
		, m_PxStatic(nullptr)
		, m_Shape(nullptr)
	{
		 
	}
	virtual			~HeightMap() = default;
	virtual void	Delete() override;

	void			AddMapPickingWrok(std::function<void(const physx::PxVec3& pickingPos)> func);
	void			ClearMapPickingWork();

private:
	virtual void	Update(float delta) override {}
	virtual void	Init(PhysX4_1* pdx, IGraphicDevice* gd) override;
	virtual void*	GetPxObject() override { return m_PxStatic; }

private:
	void LoadRAWFile(const std::wstring& filePath, int& fileHeight, int& fileWidth, std::vector<int>& datas);
	void CreateRigidStatic(PhysX4_1* pxd, int fileHeight, int fileWidth, const std::vector<int>& datas, std::vector<float>& heights);
	void CreateRenderMesh(IGraphicDevice* gd, int fileHeight, int fileWidth, const std::vector<float>& heights);
		 
	void StartMapPickingWork();

private:
	static std::function<const physx::PxVec3 & (void)>		m_GetPickingPosFunc;

	physx::PxVec3											m_Scale;
	std::wstring											m_filePath;
	std::wstring											m_fileName;

	std::vector<std::function<void(const physx::PxVec3&)>>	m_MapPickingWorks;
	std::unique_ptr<PhysXFunctionalObject>					m_Funcs;
	physx::PxRigidStatic*									m_PxStatic;
	physx::PxShape*											m_Shape;
};