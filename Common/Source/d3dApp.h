#pragma once
#include "IGraphicDevice.h"
#include "IPhysicsDevice.h"
#include "IComponent.h"
#include "CGHScene.h"
#include <Mouse.h>
#include <Keyboard.h>
#include "cCamera.h"

#define GETAPP D3DApp::GetApp()
#define GETKEY(T) const DirectX::Keyboard::KeyboardStateTracker* keyboard = (D3DApp::GetApp()->GetKeyBoard(T))
#define GETMOUSE(T) const DirectX::Mouse::ButtonStateTracker* mouse = (D3DApp::GetApp()->GetMouse(T))
#define HOLDCANCLE(T) D3DApp::GetApp()->InputDeviceHoldCancle(T)

typedef DirectX::Keyboard::Keys KEYState;
typedef DirectX::Mouse::ButtonStateTracker::ButtonState MOUSEState;

struct MeshObject;

class D3DApp
{
public:
	const std::wstring MainFolderFath = L"./../Common/";

	std::vector<std::wstring>		m_TargetMeshFolders =
	{
		MainFolderFath + L"MeshData"
	};
	std::vector<std::wstring>		m_TargetTextureFolders =
	{
		MainFolderFath + L"TextureData"
	};
	std::vector<std::wstring>		m_TargetFontFolders =
	{
		MainFolderFath + L"FontData"
	};

protected:
	D3DApp(HINSTANCE hInstance);
	D3DApp(const D3DApp& rhs) = delete;
	D3DApp& operator=(const D3DApp& rhs) = delete;
	virtual ~D3DApp();

public:
	static D3DApp* GetApp();
	const DirectX::Keyboard::KeyboardStateTracker* GetKeyBoard(const GameObject* caller);
	const DirectX::Mouse::ButtonStateTracker* GetMouse(const GameObject* caller);
	void InputDeviceHoldRequest(const GameObject* caller);
	void InputDeviceHoldCancle(const GameObject* caller);
	void GetMouseRay(DirectX::XMFLOAT3& origin, DirectX::XMFLOAT3& ray) const { origin = m_RayOrigin; ray = m_Ray; }
	DirectX::XMVECTOR XM_CALLCONV GetMousePos() const { return m_Camera.GetMousePos(); }
	DirectX::XMVECTOR XM_CALLCONV GetClientSize() const { return m_GDevice->GetClientSize(); }

	bool Initialize();
	int Run();

public:
	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

protected:
	virtual void Update(unsigned long long delta) = 0;
	virtual void SelectDevices() = 0;
	virtual void InitObjects() = 0;
	virtual bool InitMainWindow();
	
	template<typename GraphicDeviceClass, typename PhysicsDeviceClass>
	void SelectDeviceByTemplate();

	void SetCurrScene(CGHScene* scene) { m_CurrScene = scene; }
private:
	void BaseUpdate();

protected:
	static D3DApp*						m_App;

	cCamera								m_Camera;

	DirectX::Mouse						m_Mouse;
	DirectX::Mouse::ButtonStateTracker	m_MouseTracker;

	DirectX::Keyboard						m_Keyboard;
	DirectX::Keyboard::KeyboardStateTracker	m_KeyboardTracker;
	const GameObject*						m_CurrInputDeviceHoldObject;

	DirectX::XMFLOAT3	m_RayOrigin = {};
	DirectX::XMFLOAT3	m_Ray = {};

	HINSTANCE	m_hAppInst = nullptr;
	HWND		m_hMainWnd = nullptr;
	bool		m_AppPaused = false;
	bool		m_Resizing = false;
	bool		m_Minimized = false;
	bool		m_Maximized = false;

	std::unique_ptr<IGraphicDevice>	m_GDevice;
	std::unique_ptr<IPhysicsDevice>	m_PXDevice;

private:
	CGHScene*						m_CurrScene;
	unsigned long long				m_PrevTick;
	unsigned long long				m_DeltaTick;
};

template<typename GraphicDeviceClass,typename PhysicsDeviceClass>
inline void D3DApp::SelectDeviceByTemplate()
{
	assert(!m_GDevice.get());
	assert(!m_PXDevice.get());

	GraphicDeviceClass* gdeviceInstance = new GraphicDeviceClass();
	std::unique_ptr<IGraphicDevice> temp1(static_cast<IGraphicDevice*>(gdeviceInstance));
	m_GDevice = move(temp1);

	PhysicsDeviceClass* pxdeviceInstance = new PhysicsDeviceClass();
	std::unique_ptr<IPhysicsDevice> temp2(static_cast<IPhysicsDevice*>(pxdeviceInstance));
	m_PXDevice = move(temp2);
}
