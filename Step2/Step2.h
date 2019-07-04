#pragma once
#include "../Common/Source/PhysX4_0.h"
#include "../Common/Source/GraphicDX12.h"
#include "../Common/Source/d3dApp.h"
#include "../Common/Source/cCamera.h"

inline MAT16 GetDXMatrixAtRigidActor(const physx::PxRigidActor* rigidActor)
{
	MAT16 T;
	physx::PxMat44 mat(rigidActor->getGlobalPose());

	memcpy(T.m[0], &mat.column0.x, sizeof(float) * 4);
	memcpy(T.m[1], &mat.column1.x, sizeof(float) * 4);
	memcpy(T.m[2], &mat.column2.x, sizeof(float) * 4);
	memcpy(T.m[3], &mat.column3.x, sizeof(float) * 4);

	return T;
}

class Step2 final : public D3DApp
{
public:
	Step2(HINSTANCE hInstance);
	Step2(const Step2& rhs) = delete;
	Step2& operator=(const Step2& rhs) = delete;
	virtual ~Step2();
	virtual bool Initialize() override;
	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) override;

private:
	virtual void Update() override;
	virtual bool InitDrawDevice() override;
	virtual bool InitPyhsicsDevice() override;

private:
	void InitObjects();

private:
	cCamera	m_camera;
};

