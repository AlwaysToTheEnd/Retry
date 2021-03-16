#include "Step2.h"
#include <random>
#include <DirectXColors.h>
#include "PhysX4_1.h"
#include "PhysicsDO.h"
#include "AnimationTreeScene/AnimationTreeScene.h"
#include "HeightMapCreateScene/HeightMapCreateScene.h"
using namespace std;

mt19937_64 g_random(710);

Step2::Step2(HINSTANCE hInstance)
	: D3DApp(hInstance)
{
	testScene = nullptr;
}

Step2::~Step2()
{
	delete testScene;
}

void Step2::Update(float delta)
{
	
}

void Step2::SelectDevices()
{
	SelectGDeviceByTemplate<UsingGraphicDevice>();
}

void Step2::LoadObjectsFromFile()
{

}

void Step2::InitObjects()
{
	 testScene = new AniTreeScene(m_GDevice.get(), m_SDevice.get(), m_PXDevice);
	 testScene->Init();
	 SetCurrScene(testScene);
}

/////////////////////////////////////////////////////////////////////////////////////////

LRESULT Step2::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
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
	catch (DxException e)
	{
		MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
		return 0;
	}
}