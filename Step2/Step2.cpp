#include "Step2.h"
#include <random>
#include "BaseComponent.h"
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
}

void Step2::SelectDevices()
{
	SelectDeviceByTemplate<UsingGraphicDevice, UsingPhsicsDevice>();
}

void Step2::InitObjects()
{
	m_GDevice->SetCamera(&m_Camera);
}

/////////////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<IComponent> Step2::CreateComponent(COMPONENTTYPE type, GameObject& gameObject)
{
	if (type == COMPONENTTYPE::COM_END)
	{
		assert(false && "THIS COMPONENT IS NONE USED TYPE");
		return nullptr;
	}

	if (type == COMPONENTTYPE::COM_TRANSFORM)
	{
		return make_unique<ComTransform>(gameObject, 
			static_cast<unsigned int>(m_ObjectsMat.size()), &m_ObjectsMat);

		m_ObjectsMat.emplace_back();
	}
	if (type < COMPONENTTYPE::COM_TRANSFORM)
	{
		return m_PXDevice->CreateComponent(type, gameObject);
	}
	else
	{
		return m_GDevice->CreateComponent(type, gameObject);
	}

	return nullptr;
}

LRESULT Step2::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	m_Camera.WndProc(hwnd, msg, wParam, lParam);
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