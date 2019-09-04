#pragma once
#include "d3dUtil.h"
#include "IComponentProvider.h"

class cCamera;
using namespace CGH;

struct SubmeshData
{
	void*	data = nullptr;
	UINT	byteSize = 0;
	UINT	strideSize = 0;
	std::string subMeshName;
};

class IGraphicDevice : public ICompnentProvider
{
public:
	IGraphicDevice() = default;
	virtual ~IGraphicDevice() = default;

	virtual bool Init(HWND hWnd) = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;
	virtual void* GetDevicePtr() = 0;
	virtual void OnResize() = 0;

public:
	virtual void SetCamera(cCamera* camera) = 0;
	virtual void AddMesh(UINT numSubResource, SubmeshData subResources[]) = 0;

public:
	void SetClientSize(UINT width, UINT height) { m_ClientWidth = width, m_ClientHeight = height; }

protected:
	MAT16			m_ViewMatrix;
	MAT16			m_ProjectionMat;
	HWND			m_MainWndHandle = nullptr;
	int				m_ClientWidth = 1000;
	int				m_ClientHeight = 700;
};

