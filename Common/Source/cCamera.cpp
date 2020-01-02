#include "cCamera.h"
#include "foundation/PxMathUtils.h"
#include <DirectXMath.h>

using namespace physx;
using namespace DirectX;

cCamera::cCamera()
	: m_ViewMat(physx::PxIDENTITY::PxIdentity)
	, m_Angles(0,0)
	, m_EyePos(PxIdentity)
	, m_CurrMouse(PxIdentity)
	, m_PrevMouse(PxIdentity)
	, m_Distance(10)
	, m_IsRButtonDown(false)
{
}


cCamera::~cCamera()
{

}

void cCamera::Update()
{
	XMVECTOR eyePos = XMVectorSet(0, 0, -m_Distance, 1);

	eyePos = XMVector3TransformCoord(eyePos, XMMatrixRotationRollPitchYawFromVector(XMVectorSet(m_Angles.y, m_Angles.x,0,0)));
	XMVECTOR lookAt = XMVectorZero();

	/*if (pTarget)
	{
		lookAt = XMLoadFloat3(pTarget);
		eyePos = XMLoadFloat3(pTarget) + eyePos;
	}*/

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

PxVec3 cCamera::GetViewRay(const physx::PxMat44& projectionMat, unsigned int viewPortWidth, unsigned int viewPortHeight) const
{
	return PxVec3((2.0f * m_CurrMouse.x / viewPortWidth - 1.0f) / projectionMat[0][0],
		(-2.0f * m_CurrMouse.y / viewPortHeight + 1.0f) / projectionMat[1][1], 1.0f);
}
