#include "cCamera.h"
using namespace DirectX;

cCamera::cCamera()
	: m_rotX(0)
	, m_rotY(0)
	, m_distance(5)
{
	m_viewMat.Identity();
}


cCamera::~cCamera()
{
}

void cCamera::Update()
{
	XMVECTOR target= XMVectorZero();
	
	XMMATRIX rotationMat = XMMatrixRotationRollPitchYaw(0, m_rotX, m_rotY);

	XMVECTOR eyePos = XMVector3TransformNormal(XMVectorSet( 0 ,0 ,m_distance, 0), rotationMat);
	eyePos = target - eyePos;

	rotationMat = XMMatrixLookAtLH(eyePos, target, XMVectorSet(0, 1, 0, 0));
	
	XMStoreFloat4x4(m_viewMat, rotationMat);
}


void cCamera::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_RBUTTONDOWN:
	{
		m_prevMouse.x = LOWORD(lParam);
		m_prevMouse.y = HIWORD(lParam);
		m_isRButtonDown = true;
	}
	break;
	case WM_RBUTTONUP:
	{
		m_isRButtonDown = false;
	}
	break;
	case WM_MOUSEMOVE:
	{
		if (m_isRButtonDown)
		{
			POINT ptCurrMouse;
			ptCurrMouse.x = LOWORD(lParam);
			ptCurrMouse.y = HIWORD(lParam);

			m_rotX += (ptCurrMouse.y - m_prevMouse.y) / 100.0f;
			m_rotY += (ptCurrMouse.x - m_prevMouse.x) / 100.0f;

			if (m_rotX >= XM_PI * 0.5f - 0.1f)
				m_rotX = XM_PI * 0.5f - 0.1f;
			else if (m_rotX <= -XM_PI * 0.5f + 0.1f)
				m_rotX = -XM_PI * 0.5f + 0.1f;

			m_prevMouse = ptCurrMouse;
		}
	}
	break;
	case WM_MOUSEWHEEL:
	{
		m_distance -= GET_WHEEL_DELTA_WPARAM(wParam) / 100.0f;
	}
	break;
	}
}