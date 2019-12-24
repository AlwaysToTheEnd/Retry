#include "Step2.h"
#include <random>
#include "../Common/UIObjects/UIParam.h"
#include "BaseComponent.h"
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

void Step2::Update()
{
	
}

void Step2::SelectDevices()
{
	SelectDeviceByTemplate<UsingGraphicDevice, UsingPhsicsDevice>();
}

void Step2::InitObjects()
{
	 testScene = new CGHScene(m_GDevice.get(), m_PXDevice.get(), "testScene");
	 SetCurrScene(testScene);

	 static float testfloat = 23.67f;
	 auto test = new UIParam(*testScene);
	 testScene->AddGameObjects(test);

	 test->SetTargetParam(L"TestParam", &testfloat);
	 test->SetTextHeight(15);
	 test->GetComponent<ComTransform>()->SetTransform(physx::PxTransform(150, 150, 0));
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