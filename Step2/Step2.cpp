#include "Step2.h"
#include <random>
#include "XFileParser.h"
using namespace std;

mt19937_64 g_random(710);

Step2::Step2(HINSTANCE hInstance)
	: D3DApp(hInstance)
{

}

Step2::~Step2()
{
	
}

void Step2::Update()
{
	m_Camera.Update();
}

void Step2::SelectDevices()
{
	SelectDeviceByTemplate<UsingGraphicDevice, UsingPhsicsDevice>();

	XFileParser test("./../Common/TeraResourse/Poalong.X");
}

void Step2::InitObjects()
{
	m_GDevice->SetCamera(&m_Camera);
}

/////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<IComponent> Step2::CreateComponent(COMPONENTTYPE type, PxTransform& tran)
{
	switch (type)
	{
	case COMPONENTTYPE::COM_PHYSICS:
		return m_PXDevice->CreateComponent(tran);
	case COMPONENTTYPE::COM_GRAPHIC:
		return m_GDevice->CreateComponent(tran);
	case COMPONENTTYPE::COM_NONE:
	default:
		break;
	}

	assert(false);
	return nullptr;
}

LRESULT Step2::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	m_Camera.WndProc(hwnd, msg, wParam, lParam);
	m_Mouse.ProcessMessage(msg, wParam, lParam);
	m_Keyboard.ProcessMessage(msg, wParam, lParam);

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