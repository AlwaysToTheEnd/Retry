#include "d3dApp.h"
#include <WindowsX.h>

using namespace std;

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    return D3DApp::GetApp()->MsgProc(hwnd, msg, wParam, lParam);
}

D3DApp* D3DApp::m_App = nullptr;
D3DApp* D3DApp::GetApp()
{
    return m_App;
}

D3DApp::D3DApp(HINSTANCE hInstance)
:	m_hAppInst(hInstance)
{
    assert(m_App == nullptr);
    m_App = this;
}

D3DApp::~D3DApp()
{
}

int D3DApp::Run()
{
	MSG msg = {};

	while(msg.message != WM_QUIT)
	{
		if(PeekMessage( &msg, 0, 0, 0, PM_REMOVE ))
		{
            TranslateMessage( &msg );
            DispatchMessage( &msg );
		}
		else
        {	
			if( !m_appPaused )
			{
				Update();	
				m_GDevice->Draw();
			}
			else
			{
				Sleep(100);
			}
        }
    }

	return (int)msg.wParam;
}

bool D3DApp::Initialize()
{
	if(!InitMainWindow())
		return false;

	if(!InitDrawDevice())
		return false;

	return true;
}
 
 
LRESULT D3DApp::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch( msg )
	{
	case WM_ACTIVATE:
		if( LOWORD(wParam) == WA_INACTIVE )
		{
			m_appPaused = true;
		}
		else
		{
			m_appPaused = false;
		}
		return 0;
	
	case WM_ENTERSIZEMOVE:
		m_appPaused = true;
		return 0;

	case WM_EXITSIZEMOVE:
		m_appPaused = false;
		return 0;
 
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_MENUCHAR:
        return MAKELRESULT(0, MNC_CLOSE);

	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200; 
		return 0;

    case WM_KEYUP:
        if(wParam == VK_ESCAPE)
        {
            PostQuitMessage(0);
        }

        return 0;
	}
	
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

bool D3DApp::InitDrawDevice()
{

#ifdef CURRENT_DEVICE_DX12
	m_GDevice = make_unique<GraphicDX12>();
#endif

	return m_GDevice->Init(m_hMainWnd);
}

bool D3DApp::InitMainWindow()
{
	WNDCLASS wc;
	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = MainWndProc; 
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = m_hAppInst;
	wc.hIcon         = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor       = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName  = 0;
	wc.lpszClassName = L"MainWnd";

	if( !RegisterClass(&wc) )
	{
		MessageBox(0, L"RegisterClass Failed.", 0, 0);
		return false;
	}

	RECT R = { 0, 0, winWidth, winHeight };
    AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width  = R.right - R.left;
	int height = R.bottom - R.top;

	m_hMainWnd = CreateWindow(L"MainWnd", L"Test", 
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, m_hAppInst, 0); 
	if( !m_hMainWnd )
	{
		MessageBox(0, L"CreateWindow Failed.", 0, 0);
		return false;
	}

	ShowWindow(m_hMainWnd, SW_SHOW);
	UpdateWindow(m_hMainWnd);

	return true;
}
