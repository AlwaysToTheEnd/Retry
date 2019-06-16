#pragma once
#include "d3dUtil.h"

using namespace CGH;

class GraphicDevice
{
public:
	GraphicDevice() = default;
	virtual ~GraphicDevice() = default;
	virtual bool Init(HWND hWnd) = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;

	virtual void OnResize()=0;

protected:
	MAT16 m_ViewMatrix;
	MAT16 m_ProjectionMat;
};

