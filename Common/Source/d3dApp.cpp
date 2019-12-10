#include "d3dApp.h"
#include "BaseComponent.h"
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
	: m_hAppInst(hInstance)
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

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			if (!m_AppPaused)
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

std::unique_ptr<IComponent> D3DApp::CreateComponent(COMPONENTTYPE type, GameObject& gameObject)
{
	unique_ptr<IComponent> result = nullptr;

	assert(type != COMPONENTTYPE::COM_END && "THIS COMPONENT IS NONE USED TYPE");

	auto ComUpdater = GetComponentUpdater(type);
	UINT id = ComUpdater.GetNextID();

	if (type == COMPONENTTYPE::COM_TRANSFORM)
	{
		result = make_unique<ComTransform>(gameObject, id, m_ObjectsMat.GetDataPtr());

		m_ObjectsMat.AddData();
	}
	if (type < COMPONENTTYPE::COM_TRANSFORM)
	{
		return m_PXDevice->CreateComponent(type, gameObject);
	}
	else
	{
		return m_GDevice->CreateComponent(type, gameObject);
	}

	ComUpdater.AddData(result.get());
	return result;
}

void D3DApp::ComponentDeleteManaging(COMPONENTTYPE type, int id)
{
	if (type == COMPONENTTYPE::COM_END)
	{
		assert(false && "THIS COMPONENT IS NONE USED TYPE");
	}

	auto ComUpdater = GetComponentUpdater(type);
	ComUpdater.SignalDelete(id);

	if (type == COMPONENTTYPE::COM_TRANSFORM)
	{
		m_ObjectsMat.SignalDelete(id);
	}
	if (type < COMPONENTTYPE::COM_TRANSFORM)
	{
		m_PXDevice->ComponentDeleteManaging(type, id);
	}
	else
	{
		m_GDevice->ComponentDeleteManaging(type, id);
	}
}

bool D3DApp::Initialize()
{
	if (!InitMainWindow()) return false;

	SelectDevices();
	IComponent::SetInstanceDeleteManagingFunc(bind(&D3DApp::ComponentDeleteManaging, this, placeholders::_1, placeholders::_2));

	if (!m_GDevice->Init(m_hMainWnd)) return false;
	if (!m_PXDevice->Init(m_GDevice->GetDevicePtr())) return false;

	m_GDevice->ReadyWorks(m_TargetTextureFolders, m_TargetMeshFolders);

	InitObjects();

	return true;
}


LRESULT D3DApp::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	m_Mouse.ProcessMessage(msg, wParam, lParam);
	m_Keyboard.ProcessMessage(msg, wParam, lParam);

	switch (msg)
	{
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
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
		if (wParam == VK_ESCAPE)
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
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_hAppInst;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = L"MainWnd";

	if (!RegisterClass(&wc))
	{
		MessageBox(0, L"RegisterClass Failed.", 0, 0);
		return false;
	}

	RECT R = { 0, 0, 700, 700 };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width = R.right - R.left;
	int height = R.bottom - R.top;

	m_hMainWnd = CreateWindow(L"MainWnd", L"Test",
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, m_hAppInst, 0);
	if (!m_hMainWnd)
	{
		MessageBox(0, L"CreateWindow Failed.", 0, 0);
		return false;
	}

	ShowWindow(m_hMainWnd, SW_SHOW);
	UpdateWindow(m_hMainWnd);

	return true;
}
