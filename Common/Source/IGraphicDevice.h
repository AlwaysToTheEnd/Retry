#pragma once
#include "d3dUtil.h"
#include "IComponentProvider.h"
#include "Vertex.h"

class cCamera;
using namespace CGH;

namespace CGH
{
	enum MESH_TYPE
	{
		MESH_NORMAL,
		MESH_SKINED
	};
}

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

	void ReadyWorks()
	{
		LoadMeshAndMaterialFromFolder();
		LoadTextureFromFolder();
	}

public:
	virtual void SetCamera(cCamera* camera) = 0;

public:
	void SetClientSize(UINT width, UINT height) { m_ClientWidth = width, m_ClientHeight = height; }

protected:
	virtual void LoadMeshAndMaterialFromFolder() = 0;
	virtual void LoadTextureFromFolder() = 0;

protected:
	MAT16			m_ViewMatrix;
	MAT16			m_ProjectionMat;
	HWND			m_MainWndHandle = nullptr;
	int				m_ClientWidth = 1000;
	int				m_ClientHeight = 700;
};

