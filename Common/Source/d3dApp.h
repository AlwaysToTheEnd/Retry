#pragma once
#include "d3dUtil.h"
#include "IGraphicDevice.h"
#include "IPhysicsDevice.h"
#include "IComponent.h"
#include "DirectXTK/Mouse.h"
#include "DirectXTK/Keyboard.h"

#define GETAPP D3DApp::GetApp()
#define GKEYBOARD D3DApp::GetApp()->GetKeyBoard()
#define GMOUSE D3DApp::GetApp()->GetMouse()
extern const std::string gMainFolderFath = "./../Common/";

struct MeshObject;

class D3DApp
{
public:
	std::vector<std::string>		m_TargetMeshFolders =
	{
		gMainFolderFath + "MeshData"
	};
	std::vector<std::string>		m_TargetTextureFolders =
	{
		gMainFolderFath + "TextureData"
	};

protected:
	D3DApp(HINSTANCE hInstance);
	D3DApp(const D3DApp& rhs) = delete;
	D3DApp& operator=(const D3DApp& rhs) = delete;
	virtual ~D3DApp();

public:
	static D3DApp* GetApp();
	Mouse* GetMouse() { return &m_Mouse; }
	Keyboard* GetKeyBoard() { return &m_Keyboard; }

	bool Initialize();
	int Run();

public:
	virtual std::unique_ptr<IComponent> CreateComponent(COMPONENTTYPE type, GameObject& gameObject) = 0;
	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

protected:
	virtual void Update() = 0;
	virtual void SelectDevices() = 0;
	virtual void InitObjects() = 0;
	virtual bool InitMainWindow();

	template<typename GraphicDeviceClass, typename PhysicsDeviceClass>
	void SelectDeviceByTemplate();

private:
	void BaseUpdate();

protected:
	static D3DApp* m_App;

	Mouse		m_Mouse;
	Keyboard	m_Keyboard;

	HINSTANCE	m_hAppInst = nullptr;
	HWND		m_hMainWnd = nullptr;
	bool		m_AppPaused = false;
	bool		m_Resizing = false;
	bool		m_Minimized = false;
	bool		m_Maximized = false;

	std::unique_ptr<IGraphicDevice>	m_GDevice;
	std::unique_ptr<IPhysicsDevice>	m_PXDevice;
};

template<typename GraphicDeviceClass,typename PhysicsDeviceClass>
inline void D3DApp::SelectDeviceByTemplate()
{
	assert(!m_GDevice.get());
	assert(!m_PXDevice.get());

	GraphicDeviceClass* gdeviceInstance = new GraphicDeviceClass();
	std::unique_ptr<IGraphicDevice> temp1(static_cast<IGraphicDevice*>(gdeviceInstance));
	m_GDevice = move(temp1);

	PhysicsDeviceClass* pxdeviceInstance = new PhysicsDeviceClass();
	std::unique_ptr<IPhysicsDevice> temp2(static_cast<IPhysicsDevice*>(pxdeviceInstance));
	m_PXDevice = move(temp2);
}
