#pragma once
#include "d3dUtil.h"

class cCamera;
using namespace CGH;

class IGraphicDevice
{
	friend class D3DApp;
public:
	IGraphicDevice() = default;
	virtual ~IGraphicDevice() = default;

protected:
	virtual bool Init(HWND hWnd) = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;
	virtual void* GetDevicePtr() = 0;
	virtual void OnResize() = 0;

public:
	virtual void SetCamera(cCamera* camera) = 0;

public:
	void SetClientSize(UINT width, UINT height) { m_ClientWidth = width, m_ClientHeight = height; }

protected:
	MAT16			m_ViewMatrix;
	MAT16			m_ProjectionMat;
	HWND			m_MainWndHandle = nullptr;
	int				m_ClientWidth = 1000;
	int				m_ClientHeight = 700;
};

