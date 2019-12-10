#pragma once
#include "../Common/Source/PhysX4_0.h"
#include "../Common/Source/GraphicDX12.h"
#include "../Common/Source/d3dApp.h"
#include "../Common/Source/cCamera.h"

//using typedef to select core devices.
typedef GraphicDX12	UsingGraphicDevice;
typedef PhysX4_0	UsingPhsicsDevice;

inline CGH::MAT16 GetDXMatrixAtRigidActor(const physx::PxRigidActor* rigidActor)
{
	CGH::MAT16 T;
	physx::PxMat44 mat(rigidActor->getGlobalPose());

	memcpy(&T.m[0][0], &mat.column0.x, sizeof(CGH::MAT16));

	return T;
}

class Step2 final : public D3DApp
{
public:
	Step2(HINSTANCE hInstance);
	Step2(const Step2& rhs) = delete;
	Step2& operator=(const Step2& rhs) = delete;
	virtual ~Step2();

	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) override;

private:
	virtual void Update() override;
	virtual void SelectDevices() override;

private:
	virtual void InitObjects() override;

private:
	cCamera					m_Camera;
};