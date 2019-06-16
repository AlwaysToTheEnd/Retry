#pragma once
#include "d3dUtil.h"

#define CURRENT_DEVICE_DX12


#ifdef CURRENT_DEVICE_DX12
#include "GraphicDX12.h"
#endif


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
	virtual void Update()=0;
 	bool InitDrawDevice();

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

	const int winWidth = 700;
	const int winHeight = 700;

	std::unique_ptr<GraphicDevice> m_GDevice;
};

