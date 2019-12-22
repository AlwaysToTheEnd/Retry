#pragma once
#include "d3dUtil.h"
#include "IComponentCreater.h"
#include "Vertex.h"

class cCamera;
struct MeshObject;
class D3DApp;

class IGraphicDevice : public ICompnentCreater
{
	friend class D3DApp;
public:
	IGraphicDevice() = default;
	virtual ~IGraphicDevice() = default;

	virtual bool Init(HWND hWnd, UINT windowWidth, UINT windowHeight) = 0;
	virtual void Update(const CGHScene& scene) = 0;
	virtual void Draw() = 0;
	virtual void ReservedWorksClear() = 0;
	virtual void* GetDevicePtr() = 0;
	virtual void OnResize() = 0;
	virtual void GetWorldRay(DirectX::XMFLOAT3& origin, DirectX::XMFLOAT3& ray) const = 0;
	virtual void CreateScene(const CGHScene& scene) = 0;

private:
	void ReadyWorks(const std::vector<std::wstring>& targetTextureFolders,
					const std::vector<std::wstring>& targetMeshFolders,
					const std::vector<std::wstring>& targetFontFolders)
	{
		LoadTextureFromFolder(targetTextureFolders);
		LoadMeshAndMaterialFromFolder(targetMeshFolders);
		LoadFontFromFolder(targetFontFolders);
		ReadyWorksEnd();
	}

public:
	virtual void SetCamera(cCamera* camera) = 0;

public:
	void SetClientSize(UINT width, UINT height) { m_ClientWidth = width, m_ClientHeight = height; }

protected:
	virtual void LoadTextureFromFolder(const std::vector<std::wstring>& targetTextureFolders) = 0;
	virtual void LoadMeshAndMaterialFromFolder(const std::vector<std::wstring>& targetMeshFolders) = 0;
	virtual void LoadFontFromFolder(const std::vector<std::wstring>& targetFontFolders) = 0;
	virtual void ReadyWorksEnd() = 0;

protected:
	physx::PxMat44		m_ViewMatrix;
	physx::PxMat44		m_ProjectionMat;
	physx::PxMat44		m_OrthoProjectionMat;
	HWND			m_MainWndHandle = nullptr;
	int				m_ClientWidth = 700;
	int				m_ClientHeight = 700;
};

