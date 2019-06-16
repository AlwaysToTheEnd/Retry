#pragma once
#include "PxPhysicsAPI.h"
#include "../Common/Source/d3dApp.h"
#include "../Common/Source/cCamera.h"
#include "../Common/Source/Px1RefPtr.h"
using namespace physx;

#define APPMAIN static_cast<Step2*>(D3DApp::GetApp())


inline MAT16 GetDXMatrixAtRigidActor(const PxRigidActor* rigidActor)
{
	MAT16 T;
	PxMat44 mat(rigidActor->getGlobalPose());

	memcpy(T.m[0], &mat.column0.x, sizeof(float) * 4);
	memcpy(T.m[1], &mat.column1.x, sizeof(float) * 4);
	memcpy(T.m[2], &mat.column2.x, sizeof(float) * 4);
	memcpy(T.m[3], &mat.column3.x, sizeof(float) * 4);

	return T;
}

class MyGPULoadHook :public PxGpuLoadHook
{
public:
	MyGPULoadHook() = default;
	virtual ~MyGPULoadHook() = default;

	virtual const char* getPhysXGpuDllName() const override
	{
		return "../Out/PhysXGpu_64.dll";
	}
};

class Step2 final : public D3DApp
{
public:
	Step2(HINSTANCE hInstance);
	Step2(const Step2& rhs) = delete;
	Step2& operator=(const Step2& rhs) = delete;
	virtual ~Step2();
	virtual bool Initialize() override;
	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) override;

public:
	GraphicDevice* GetGDDevice() { return m_GDevice.get(); }
	PxPhysics* GetPXDevice() { return m_physics.Get(); }
	const cCamera* GetCamera() { return &m_camera; }

private:
	virtual void Update() override;

private:
	void InitPhsyX();
	void InitObjects();

private:

	void PhysXBaseUpdate();
	void MainPassUpdate();
	static PxFilterFlags ScissorFilter(PxFilterObjectAttributes attributes0, PxFilterData filterData0,
		PxFilterObjectAttributes attributes1, PxFilterData filterData1,
		PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize);

private:
	PxDefaultAllocator					m_allocator;
	PxDefaultErrorCallback				m_errorCallback;

	Px1RefPtr<PxPhysics>				m_physics;
	Px1RefPtr<PxFoundation>				m_foundation;
	Px1RefPtr<PxDefaultCpuDispatcher>	m_dispatcher;
	Px1RefPtr<PxCooking>				m_cooking;
	Px1RefPtr<PxCudaContextManager>		m_cudaManager;
	Px1RefPtr<PxScene>					m_scene;
	Px1RefPtr<PxPvd>					m_PVD;

private:
	cCamera								m_camera;

private:
	Px1RefPtr<PxMaterial>				m_planeMaterial;
	PxArticulationJointReducedCoordinate* m_driveJoint;
};

