#include "Step2.h"
#include <random>
#include "TestObject.h"
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
	 testScene->SetGetRayFunc([](DirectX::XMFLOAT3& origin, DirectX::XMFLOAT3& ray)
		 {
			 DirectX::XMFLOAT2 mousePos;
			 DirectX::XMFLOAT2 clientSize;
			 DirectX::XMStoreFloat2(&mousePos, GETAPP->GetMousePos());
			 DirectX::XMStoreFloat2(&clientSize, GETAPP->GetClientSize());
			 origin.x = mousePos.x- clientSize.x/2;
			 origin.y = -mousePos.y + clientSize.y/2;
			 origin.z = -1;

			 ray = { 0, 0, 1.0f };
		 });

	 testScene->AddGameObjects(new TestObject(*testScene));
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