#include "Step2.h"
#include <random>
using namespace std;

mt19937_64 g_random(710);

Step2::Step2(HINSTANCE hInstance)
	: D3DApp(hInstance)
{

}

Step2::~Step2()
{
	
}

bool Step2::Initialize()
{
	if (!D3DApp::Initialize())
	{
		return false;
	}

	InitObjects();

	return true;
}

void Step2::Update()
{
	m_camera.Update();
	m_PXDevice->Update();
	m_GDevice->Update(m_camera);
}

bool Step2::InitDrawDevice()
{
	m_GDevice = make_unique<GraphicDX12>();

	return m_GDevice->Init(m_hMainWnd);
}

bool Step2::InitPyhsicsDevice()
{
	m_PXDevice = make_unique<PhysX4_0>();
	
	return m_PXDevice->Init();
}

void Step2::InitObjects()
{
	
}

/////////////////////////////////////////////////////////////////////////////////////////

LRESULT Step2::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	m_camera.WndProc(hwnd, msg, wParam, lParam);

	return D3DApp::MsgProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, PSTR, int)
{
#if defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	try
	{
		Step2 theApp(hInstance);
		if (!theApp.Initialize())
			return 0;

		return theApp.Run();
	}
	catch (std::exception e)
	{
		std::string stringMessage(e.what());
		std::wstring exceptionMessage(stringMessage.begin(), stringMessage.end());
		MessageBox(nullptr, exceptionMessage.c_str(), L"HR Failed", MB_OK);
		return 0;
	}
}