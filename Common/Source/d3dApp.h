#pragma once
#include "d3dUtil.h"
#include "GraphicDevice.h"
#include "PhysicsDevice.h"


class D3DApp
{
protected:
    D3DApp(HINSTANCE hInstance);
    D3DApp(const D3DApp& rhs) = delete;
    D3DApp& operator=(const D3DApp& rhs) = delete;
    virtual ~D3DApp();

public:
    static D3DApp* GetApp();
    
	virtual bool Initialize();
    virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	int Run();

protected:
	virtual void Update() = 0;
 	virtual bool InitDrawDevice() = 0;
	virtual bool InitPyhsicsDevice() = 0;

protected:
	bool InitMainWindow();

protected:
    static D3DApp* m_App;

    HINSTANCE	m_hAppInst = nullptr;
    HWND		m_hMainWnd = nullptr;
	bool		m_AppPaused = false;  
	bool		m_Resizing = false;
	bool		m_Minimized = false;
	bool		m_Maximized = false;

	std::unique_ptr<GraphicDevice>	m_GDevice;
	std::unique_ptr<PhysicsDevice>	m_PXDevice;
};

