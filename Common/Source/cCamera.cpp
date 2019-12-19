#include "cCamera.h"
using namespace DirectX;

cCamera::cCamera()
	: m_RotX(0)
	, m_RotY(0)
	, m_Distance(5)
	, m_EyePos(0,0,0)
{
	m_ViewMat.Identity();
}


cCamera::~cCamera()
{
}

void cCamera::Update()
{
	XMVECTOR target = XMLoadFloat3(&m_EyePos);
	
	XMMATRIX viewMat = XMMatrixRotationRollPitchYaw(0, m_RotX, m_RotY);

	XMVECTOR eyePos = XMVector3TransformNormal(XMVectorSet( 0 ,0 ,m_Distance, 0), viewMat);
	eyePos = target - eyePos;

	viewMat = XMMatrixLookAtLH(eyePos, target, XMVectorSet(0, 1, 0, 0));
	
	XMStoreFloat4x4(m_ViewMat, viewMat);
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
			m_RotX += (m_CurrMouse.y - m_PrevMouse.y) / 100.0f;
			m_RotY += (m_CurrMouse.x - m_PrevMouse.x) / 100.0f;

			if (m_RotX >= XM_PI * 0.5f - 0.1f)
				m_RotX = XM_PI * 0.5f - 0.1f;
			else if (m_RotX <= -XM_PI * 0.5f + 0.1f)
				m_RotX = -XM_PI * 0.5f + 0.1f;

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

DirectX::XMVECTOR XM_CALLCONV cCamera::GetViewRay(const CGH::MAT16& projectionMat, unsigned int viewPortWidth, unsigned int viewPortHeight) const
{
	return DirectX::XMVectorSet(
		((m_CurrMouse.x * 2.0f) / viewPortWidth - 1.0f) / projectionMat.m[0][0],
		((-m_CurrMouse.y * 2.0f) / viewPortHeight + 1.0f) / projectionMat.m[1][1],
		1.0f, 0.0f);
}
