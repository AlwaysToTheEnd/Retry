#include "d3dApp.h"
#include "PhysX4_1.h"
#include "PhysicsDO.h"
#include "StaticObject.h"
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

const DirectX::Keyboard::KeyboardStateTracker* D3DApp::GetKeyBoard(const GameObject* const caller)
{
	const DirectX::Keyboard::KeyboardStateTracker* result = nullptr;

	if (caller && caller == m_CurrInputDeviceHoldObject)
	{
		result = &m_KeyboardTracker;
	}

	return result;
}

const DirectX::Mouse::ButtonStateTracker* D3DApp::GetMouse(const GameObject* const caller)
{
	const DirectX::Mouse::ButtonStateTracker* result = nullptr;

	if (caller && caller == m_CurrInputDeviceHoldObject)
	{
		result = &m_MouseTracker;
	}

	return result;
}

void D3DApp::InputDeviceHoldRequest(const GameObject* const caller)
{
	if (caller)
	{
		m_CurrInputDeviceHoldObject = caller;
	}
}

void D3DApp::InputDeviceHoldCancle(const GameObject* const caller)
{
	if (caller)
	{
		if (m_CurrInputDeviceHoldObject == caller)
		{
			m_CurrInputDeviceHoldObject = nullptr;
		}
	}
}

D3DApp::D3DApp(HINSTANCE hInstance)
	: m_hAppInst(hInstance)
	, m_CurrScene(nullptr)
	, m_CurrInputDeviceHoldObject(nullptr)
	, m_PXDevice(nullptr)
{
	assert(m_App == nullptr);
	m_App = this;
}

D3DApp::~D3DApp()
{
	if (m_PXDevice)
	{
		delete m_PXDevice;
	}
}

void D3DApp::CreatePhysxDevice()
{
	m_PXDevice = new PhysX4_1;
}

void D3DApp::CameraMove()
{
	if (m_CurrInputDeviceHoldObject == nullptr)
	{
		float baseSpeed = 20.0f;
		if (m_KeyboardTracker.lastState.LeftShift)
		{
			baseSpeed *= 5;
		}

		if (m_KeyboardTracker.lastState.W)
		{
			m_Camera.MoveCamera(cCamera::CAMERA_MOVE_DIR::DIR_FRONT, m_Timer.DeltaTime()* baseSpeed);
		}
		else if (m_KeyboardTracker.lastState.S)
		{
			m_Camera.MoveCamera(cCamera::CAMERA_MOVE_DIR::DIR_BACK, m_Timer.DeltaTime()* baseSpeed);
		}

		if (m_KeyboardTracker.lastState.D)
		{
			m_Camera.MoveCamera(cCamera::CAMERA_MOVE_DIR::DIR_RIGHT, m_Timer.DeltaTime()* baseSpeed);
		}
		else if (m_KeyboardTracker.lastState.A)
		{
			m_Camera.MoveCamera(cCamera::CAMERA_MOVE_DIR::DIR_LEFT, m_Timer.DeltaTime()* baseSpeed);
		}
	}
}

void D3DApp::BaseUpdate()
{
	m_MouseTracker.Update(m_Mouse.GetState());
	m_KeyboardTracker.Update(m_Keyboard.GetState());
	CameraMove();
	m_Camera.Update();

	if (m_KeyboardTracker.IsKeyPressed(KEYState::Escape))
	{
		StaticGameObjectController::WorkALLEnd();
		m_CurrInputDeviceHoldObject = nullptr;
	}

	StaticGameObjectController::StaticsUpdate(m_Timer.DeltaTime());
	Update(m_Timer.DeltaTime());

	if (m_CurrScene)
	{
		if (!m_CurrScene->Update(m_MouseTracker, m_Timer.DeltaTime()))
		{
			m_CurrInputDeviceHoldObject = nullptr;
			StaticGameObjectController::WorkAllClear();
		}
	}

	m_GDevice->GetWorldRay(m_RayOrigin, m_Ray);
}

void D3DApp::CalculateFrame()
{
	static int frameCnt = 0;
	static float timeElapsed = 0.0f;

	frameCnt++;

	if ((m_Timer.TotalTime() - timeElapsed) >= 1.0f)
	{
		float fps = (float)frameCnt; 
		float mspf = 1000.0f / fps;

		wstring fpsStr = to_wstring(fps);
		wstring mspfStr = to_wstring(mspf);

		wstring windowText = wstring(L"누가죽나 해보자") +
			L"    fps: " + fpsStr +
			L"   mspf: " + mspfStr;

		SetWindowText(m_hMainWnd, windowText.c_str());

		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}

int D3DApp::Run()
{
	MSG msg = {};

	m_Timer.Reset();

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			m_Timer.Tick();

			if (!m_AppPaused)
			{
				CalculateFrame();
				BaseUpdate();

				m_GDevice->Draw();
				m_GDevice->ReservedWorksClear();
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
	if (!InitMainWindow()) return false;

	SelectDevices();
	m_GDevice->SetCamera(&m_Camera);

	if (!m_GDevice->Init(m_hMainWnd, DEFAULTWINDOWSIZE, DEFAULTWINDOWSIZE)) return false;
	if (!m_PXDevice->Init(m_GDevice->GetDevicePtr())) return false;

	m_GDevice->ReadyWorks(	m_TargetTextureFolders, m_TargetMeshFolders,
							m_TargetFontFolders, m_TargetAniTreeFolder);

	LoadObjectsFromFile();
	InitObjects();

	m_Timer.Start();
	return true;
}


LRESULT D3DApp::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	DirectX::Keyboard::ProcessMessage(msg, wParam, lParam);
	DirectX::Mouse::ProcessMessage(msg, wParam, lParam);
	m_Camera.WndProc(hwnd, msg, wParam, lParam);

	switch (msg)
	{
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			m_AppPaused = true;
			m_Timer.Stop();
		}
		else
		{
			m_AppPaused = false;
			m_Timer.Start();
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
					m_GDevice->SetClientSize(clientWidth, clientHeight);
					m_GDevice->OnResize();
				}
				else
				{

				}
			}
		}

		return 0;
	}
	case WM_ENTERSIZEMOVE:
		m_AppPaused = true;
		m_Resizing = true;
		m_Timer.Stop();
		return 0;

	case WM_EXITSIZEMOVE:
		m_AppPaused = false;
		m_Resizing = false;
		m_Timer.Start();
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

	RECT R = { 0, 0, DEFAULTWINDOWSIZE, DEFAULTWINDOWSIZE };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width = R.right - R.left;
	int height = R.bottom - R.top;

	m_hMainWnd = CreateWindow(L"MainWnd", L"Created",
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
