#include "cCamera.h"
#include "foundation/PxMathUtils.h"
#include <DirectXMath.h>

using namespace physx;
using namespace DirectX;

cCamera::cCamera()
	: m_ViewMat(physx::PxIDENTITY::PxIdentity)
	, m_Angles(0, 0)
	, m_EyePos(PxIdentity)
	, m_CurrMouse(PxIdentity)
	, m_PrevMouse(PxIdentity)
	, m_Distance(10)
	, m_IsRButtonDown(false)
	, m_TargetObject(nullptr)
{
}


cCamera::~cCamera()
{
	
}

void cCamera::Update()
{
	XMVECTOR eyePos = XMVectorSet(0, 0, -m_Distance, 1);
	XMVECTOR lookAt = XMVectorZero();

	eyePos = XMVector3TransformCoord(eyePos, XMMatrixRotationRollPitchYawFromVector(XMVectorSet(m_Angles.y, m_Angles.x, 0, 0)));
	XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&m_Dir), XMVector3Normalize(-eyePos));

	if (m_TargetObject)
	{
	/*	lookAt = XMLoadFloat3(pTarget);
		eyePos = XMLoadFloat3(pTarget) + eyePos;*/
	}
	else
	{
		eyePos.m128_f32[0] += m_Offset.x;
		eyePos.m128_f32[1] += m_Offset.y;
		eyePos.m128_f32[2] += m_Offset.z;

		lookAt = XMVectorSet(m_Offset.x, m_Offset.y, m_Offset.z, 0);
	}

	XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&m_EyePos), eyePos);
	XMStoreFloat4x4(m_ViewMat, XMMatrixLookAtLH(eyePos, lookAt, { 0,1,0,0 }));
}

void cCamera::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_RBUTTONDOWN:
	{
		m_PrevMouse.x = LOWORD(lParam);
		m_PrevMouse.y = HIWORD(lParam);
		m_IsRButtonDown = true;
	}
	break;
	case WM_RBUTTONUP:
	{
		m_IsRButtonDown = false;
	}
	break;
	case WM_MOUSEMOVE:
	{
		m_CurrMouse.x = LOWORD(lParam);
		m_CurrMouse.y = HIWORD(lParam);

		if (m_IsRButtonDown)
		{
			PxVec2 movement = m_CurrMouse - m_PrevMouse;
			m_Angles += movement / 100.0f;

			if (m_Angles.y <= -PxPi * 0.5f + FLT_EPSILON)
				m_Angles.y = -PxPi * 0.5f + FLT_EPSILON;
			else if (m_Angles.y >= PxPi * 0.5f - FLT_EPSILON)
				m_Angles.y = PxPi * 0.5f - FLT_EPSILON;

			m_PrevMouse = m_CurrMouse;
		}
	}
	break;
	case WM_MOUSEWHEEL:
	{
		m_Distance -= GET_WHEEL_DELTA_WPARAM(wParam) / 100.0f;
	}
	break;
	}
}

void cCamera::MoveCamera(CAMERA_MOVE_DIR dir, float speed)
{
	if (m_TargetObject == nullptr)
	{
		switch (dir)
		{
		case cCamera::CAMERA_MOVE_DIR::DIR_FRONT:
		{
			m_Offset += m_Dir* speed;
		}
			break;
		case cCamera::CAMERA_MOVE_DIR::DIR_BACK:
		{
			m_Offset -= m_Dir * speed;
		}
			break;
		case cCamera::CAMERA_MOVE_DIR::DIR_RIGHT:
		{
			physx::PxVec3 upVec(0, 1, 0);
			physx::PxVec3 rightVec = upVec.cross(m_Dir).getNormalized();

			m_Offset += rightVec * speed;
		}
			break;
		case cCamera::CAMERA_MOVE_DIR::DIR_LEFT:
		{
			physx::PxVec3 upVec(0, 1, 0);
			physx::PxVec3 rightVec = upVec.cross(m_Dir).getNormalized();

			m_Offset -= rightVec * speed;
		}
			break;
		case cCamera::CAMERA_MOVE_DIR::DIR_ORIGIN:
		{
			m_Offset = { 0,0,0 };
		}
			break;
		default:
			break;
		}
	}
}

PxVec3 cCamera::GetViewRay(const physx::PxMat44& projectionMat, unsigned int viewPortWidth, unsigned int viewPortHeight) const
{
	return PxVec3((2.0f * m_CurrMouse.x / viewPortWidth - 1.0f) / projectionMat[0][0],
		(-2.0f * m_CurrMouse.y / viewPortHeight + 1.0f) / projectionMat[1][1], 1.0f);
}
