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

	DirectX::XMVECTOR XM_CALLCONV GetViewRay(const CGH::MAT16& projectionMat, unsigned int viewPortWidth, unsigned int viewPortHeight) const;
	const CGH::MAT16* GetViewMatrix() const { return &m_ViewMat; }
	const DirectX::XMFLOAT3 GetEyePos() const { return m_EyePos; }
private:
	CGH::MAT16			m_ViewMat;
	DirectX::XMFLOAT3	m_EyePos;
	POINT				m_CurrMouse;
	POINT				m_PrevMouse;
	bool				m_IsRButtonDown;
	float				m_Distance;
	float				m_RotX;
	float				m_RotY;
};

