#pragma once
#include <Windows.h>
#include "BaseClass.h"
#include "GameObject.h"

class cCamera
{
public:
	enum class CAMERA_MOVE_DIR
	{
		DIR_FRONT,
		DIR_BACK,
		DIR_RIGHT,
		DIR_LEFT,
		DIR_ORIGIN
	};

public:
	cCamera();
	~cCamera();

	void Update();
	void WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	void SetDistance(float distance) { m_Distance = distance; }
	void MoveCamera(CAMERA_MOVE_DIR dir, float speed);

	physx::PxVec3			GetViewRay(const physx::PxMat44& projectionMat, unsigned int viewPortWidth, unsigned int viewPortHeight) const;
	const physx::PxMat44*	GetViewMatrix() const { return &m_ViewMat; }
	physx::PxVec3			GetEyePos() const { return m_EyePos; }
	physx::PxVec2			GetMousePos() const { return m_CurrMouse; }
private:
	physx::PxMat44		m_ViewMat;
	physx::PxVec3		m_EyePos;
	physx::PxVec3		m_Offset;
	physx::PxVec3		m_Dir;
	physx::PxVec2		m_Angles;
	physx::PxVec2		m_CurrMouse;
	physx::PxVec2		m_PrevMouse;
	bool				m_IsRButtonDown;
	float				m_Distance;

	const GameObject*	m_TargetObject;
};

