#pragma once
#include <Windows.h>
#include "BaseClass.h"

class cCamera
{
public:
	cCamera();
	~cCamera();

	void Update();
	void SetRotationX(float x) { m_RotX = x; }
	void SetRotationY(float y) { m_RotY = y; }
	void SetDistance(float distance) { m_Distance = distance; }
	void WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	DirectX::XMVECTOR XM_CALLCONV GetViewRay(const physx::PxMat44& projectionMat, unsigned int viewPortWidth, unsigned int viewPortHeight) const;
	const physx::PxMat44* GetViewMatrix() const { return &m_ViewMat; }
	const DirectX::XMFLOAT3 GetEyePos() const { return m_EyePos; }
	DirectX::XMVECTOR XM_CALLCONV GetMousePos() const { return DirectX::XMVectorSet(m_CurrMouse.x, m_CurrMouse.y, 0, 0); }
private:
	physx::PxMat44		m_ViewMat;
	DirectX::XMFLOAT3	m_EyePos;
	POINT				m_CurrMouse;
	POINT				m_PrevMouse;
	bool				m_IsRButtonDown;
	float				m_Distance;
	float				m_RotX;
	float				m_RotY;
};

