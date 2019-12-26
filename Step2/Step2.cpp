#include "Step2.h"
#include <random>
#include "../Common/UIObjects/UIPanel.h"
#include "BaseComponent.h"
#include <DirectXColors.h>
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
	 static bool testInt = true;
	 auto test = new UIParam(*testScene, UIParam::UIPARAMTYPE::MODIFIER);
	 auto test2 = new UIParam(*testScene, UIParam::UIPARAMTYPE::VIEWER);
	 auto testPanel = new UIPanel(*testScene);
	 auto testPane2 = new UIPanel(*testScene);
	 auto testPane3 = new UIPanel(*testScene);
	 auto testPane4 = new UIPanel(*testScene);
	 testScene->AddGameObjects(test);
	 testScene->AddGameObjects(test2);
	 testScene->AddGameObjects(testPanel);
	 testScene->AddGameObjects(testPane2);
	 testScene->AddGameObjects(testPane3);
	 testScene->AddGameObjects(testPane4);

	 test->SetTargetParam(L"TestParam", &testfloat);
	 test->SetTextHeight(15);

	 test2->SetTargetParam(L"TestParam2", &testInt);
	 test2->SetTextHeight(15);

	 testPanel->SetPos({ 300,350 });
	 testPanel->SetBackGroundColor({0.0f,0,1.0f,1});
	 testPanel->AddUICom(10, 30, test);
	 testPanel->AddUICom(10, 50, test2);
	 testPanel->SetSize(150, 150);

	 testPane2->SetPos({ 200,350 });
	 testPane2->SetBackGroundColor({ 1.0f, 1.0f, 1.0f, 1 });
	 testPane2->SetSize(150, 150);

	 testPane3->SetPos({ 100,350 });
	 testPane3->SetBackGroundColor({ 0.5f, 0.5f, 0.5f, 1 });
	 testPane3->SetSize(150, 150);

	 testPane4->SetPos({ 0,350 });
	 testPane4->SetBackGroundColor({ 1.0f, 0, 1.0f, 1 });
	 testPane4->SetSize(150, 150);
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