#pragma once
#include "d3dUtil.h"
#include "IComponentProvider.h"
#include "Vertex.h"

class cCamera;
struct MeshObject;
class D3DApp;

class IGraphicDevice : public ICompnentProvider
{
	friend class D3DApp;
public:
	IGraphicDevice() = default;
	virtual ~IGraphicDevice() = default;

	virtual bool Init(HWND hWnd) = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;
	virtual void* GetDevicePtr() = 0;
	virtual void OnResize() = 0;

private:
	void ReadyWorks(const std::vector<std::string>& targetTextureFolders,
					const std::vector<std::string>& targetMeshFolders)
	{
		LoadTextureFromFolder(targetTextureFolders);
		LoadMeshAndMaterialFromFolder(targetMeshFolders);
		ReadyWorksEnd();
	}

public:
	virtual void SetCamera(cCamera* camera) = 0;

public:
	void SetClientSize(UINT width, UINT height) { m_ClientWidth = width, m_ClientHeight = height; }

protected:
	virtual void LoadTextureFromFolder(const std::vector<std::string>& targetTextureFolders) = 0;
	virtual void LoadMeshAndMaterialFromFolder(const std::vector<std::string>& targetMeshFolders) = 0;
	virtual void ReadyWorksEnd() = 0;

protected:
	CGH::MAT16		m_ViewMatrix;
	CGH::MAT16		m_ProjectionMat;
	HWND			m_MainWndHandle = nullptr;
	int				m_ClientWidth = 1000;
	int				m_ClientHeight = 700;
};

