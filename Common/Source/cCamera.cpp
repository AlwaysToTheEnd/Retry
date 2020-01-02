#include "cCamera.h"
#include "foundation/PxMathUtils.h"

using namespace physx;

cCamera::cCamera()
	: m_ViewMat(physx::PxIDENTITY::PxIdentity)
	, m_Rot(PxIdentity)
	, m_EyePos(PxIdentity)
	, m_CurrMouse(PxIdentity)
	, m_PrevMouse(PxIdentity)
	, m_Distance(100)
	, m_IsRButtonDown(false)
{
}


cCamera::~cCamera()
{

}

void cCamera::Update()
{
	PxVec3 eyePos(0, 0, -m_Distance);

	eyePos = m_Rot.rotate(eyePos);
	PxVec3 lookAt(PxIdentity);

	m_ViewMat = PxMat44(PxTransform(eyePos, PxShortestRotation(eyePos, lookAt)));
	m_EyePos = eyePos;
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

			PxQuat qx(PxPi * movement.x / 100.0f, PxVec3(0, 1, 0));
			PxQuat qy(PxPi * movement.y / 100.0f, PxVec3(1, 0, 0));

			m_Rot *= qx;
			m_Rot *= qy;

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
