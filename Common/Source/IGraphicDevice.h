#pragma once
#include "d3dUtil.h"
#include "IComponentProvider.h"

class cCamera;
using namespace CGH;

struct SubmeshData
{
	UINT	numVertex = 0;
	UINT	vertexOffset = 0;
	UINT	numIndex = 0;
	UINT	indexOffset = 0;
};

class MeshObject
{
public:
	std::string	m_Name;

	void*		m_VertexData = nullptr;
	UINT		m_VertexByteSize = 0;
	UINT		m_VertexDataSize = 0;

	void*		m_IndexData = nullptr;
	UINT		m_IndexDataSize = 0;
	UINT		m_IndexByteSize = 0;

	std::unordered_map<std::string, SubmeshData> m_Subs;
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

