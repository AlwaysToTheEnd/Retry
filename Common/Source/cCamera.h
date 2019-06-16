#pragma once
#include <Windows.h>
#include "BaseClass.h"

class cCamera
{
public:
	cCamera();
	~cCamera();

	void Update();
	void SetRotationX(float x) { m_rotX = x; }
	void SetRotationY(float y) { m_rotY = y; }
	void SetDistance(float distance) { m_distance = distance; }
	void WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	const CGH::MAT16* GetViewMatrix() const { return &m_viewMat; }
	const DirectX::XMFLOAT3 GetEyePos() const { return m_eyePos; }
private:
	CGH::MAT16			m_viewMat;
	DirectX::XMFLOAT3	m_eyePos;
	POINT				m_prevMouse;
	bool				m_isRButtonDown;
	float				m_distance;
	float				m_rotX;
	float				m_rotY;
};

