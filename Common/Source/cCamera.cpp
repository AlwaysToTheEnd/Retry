#include "cCamera.h"
using namespace DirectX;

cCamera::cCamera()
	: m_RotX(0)
	, m_RotY(0)
	, m_Distance(10)
	, m_EyePos(0,0,0)
	, m_ViewMat(physx::PxIDENTITY::PxIdentity)
{
}


cCamera::~cCamera()
{
}

void cCamera::Update()
{
	XMMATRIX matRot = XMMatrixRotationRollPitchYaw(m_RotX, m_RotY, 0);
	XMVECTOR eyePos = XMVectorSet(0, 0, -m_Distance, 1);

	eyePos = XMVector3TransformNormal(eyePos, matRot);
	XMVECTOR lookAt = XMVectorZero();
	XMVECTOR up = { 0,1,0,0 };

	//if (m_target)
	//{
	//	XMMATRIX targetRotation = XMMatrixRotationQuaternion(XMLoadFloat4(&m_target->GetQuaternionInstance()));
	//	up = XMVector3TransformNormal(up, targetRotation);
	//	eyePos = XMVector3TransformNormal(eyePos, targetRotation);
	//	lookAt = XMLoadFloat3(&m_target->GetWorldPos());
	//	eyePos = XMLoadFloat3(&m_target->GetWorldPos()) + eyePos;
	//}

	XMStoreFloat3(&m_EyePos, eyePos);
	XMStoreFloat4x4(m_ViewMat, XMMatrixLookAtLH(eyePos, lookAt, up));
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

DirectX::XMVECTOR XM_CALLCONV cCamera::GetViewRay(const physx::PxMat44& projectionMat, unsigned int viewPortWidth, unsigned int viewPortHeight) const
{
	return DirectX::XMVectorSet(
		((m_CurrMouse.x * 2.0f) / viewPortWidth - 1.0f) / projectionMat[0][0],
		((-m_CurrMouse.y * 2.0f) / viewPortHeight + 1.0f) / projectionMat[1][1],
		1.0f, 0.0f);
}
