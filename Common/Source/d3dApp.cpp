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

void D3DApp::BaseUpdate()
{
	Update();

	m_PXDevice->Update();
	m_GDevice->Update();
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
			if( !m_AppPaused )
			{
				BaseUpdate();	
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
	if(!InitMainWindow()) return false;

	SelectDevices();

	if (!m_GDevice->Init(m_hMainWnd)) return false;
	if (!m_PXDevice->Init(m_GDevice->GetDevicePtr())) return false;

	return true;
}
 
 
LRESULT D3DApp::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch( msg )
	{
	case WM_ACTIVATE:
		if( LOWORD(wParam) == WA_INACTIVE )
		{
			m_AppPaused = true;
		}
		else
		{
			m_AppPaused = false;
		}
		return 0;

	case WM_SIZE:
	{
		int clientWidth = LOWORD(lParam);
		int clientHeight = HIWORD(lParam);

		if (m_GDevice.get())
		{
			if (wParam == SIZE_MINIMIZED)
			{
				m_AppPaused = true;
				m_Minimized = true;
				m_Maximized = false;
			}
			else if (wParam == SIZE_MAXIMIZED)
			{
				m_AppPaused = false;
				m_Minimized = false;
				m_Maximized = true;
				m_GDevice->SetClientSize(clientWidth, clientHeight);
				m_GDevice->OnResize();
			}
			else if (wParam == SIZE_RESTORED)
			{
				if (m_Minimized)
				{
					m_AppPaused = false;
					m_Minimized = false;
					m_GDevice->SetClientSize(clientWidth, clientHeight);
					m_GDevice->OnResize();
				}
				else if (m_Maximized)
				{
					m_AppPaused = false;
					m_Maximized = false;
					m_GDevice->SetClientSize(clientWidth, clientHeight);
					m_GDevice->OnResize();
				}
				else if (m_Resizing)
				{

				}
				else
				{
					m_GDevice->SetClientSize(clientWidth, clientHeight);
					m_GDevice->OnResize();
				}
			}
		}

		return 0;
	}
	case WM_ENTERSIZEMOVE:
		m_AppPaused = true;
		m_Resizing = true;
		return 0;

	case WM_EXITSIZEMOVE:
		m_AppPaused = false;
		m_Resizing = false;
		m_GDevice->OnResize();
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
	
	RECT R = { 0, 0, 700, 700 };
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
