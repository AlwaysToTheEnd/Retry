#pragma once
#include "IGraphicDevice.h"
#include "DeviceObject.h"
#include "CGHScene.h"
#include <Mouse.h>
#include <Keyboard.h>
#include "cCamera.h"
#include "GameTimer.h"

#define GETAPP D3DApp::GetApp()
#define GETKEY(T) const DirectX::Keyboard::KeyboardStateTracker* keyboard = (D3DApp::GetApp()->GetKeyBoard(T))
#define GETMOUSE(T) const DirectX::Mouse::ButtonStateTracker* mouse = (D3DApp::GetApp()->GetMouse(T))
#define HOLDCANCLE(T) D3DApp::GetApp()->InputDeviceHoldCancle(T)
#define DEFAULTWINDOWSIZE 900

typedef DirectX::Keyboard::Keys KEYState;
typedef DirectX::Mouse::ButtonStateTracker::ButtonState MOUSEState;

struct MeshObject;
class PhysX4_1;

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

	const std::wstring				m_TargetAniTreeFolder = MainFolderFath + L"AniTree";

protected:
	D3DApp(HINSTANCE hInstance);
	D3DApp(const D3DApp& rhs) = delete;
	D3DApp& operator=(const D3DApp& rhs) = delete;
	virtual ~D3DApp();

public:
	static D3DApp*	GetApp();
	bool			Initialize();
	int				Run();

	void											InputDeviceHoldRequest(const GameObject* const caller);
	void											InputDeviceHoldCancle(const GameObject* const caller);
	const DirectX::Keyboard::KeyboardStateTracker*	GetKeyBoard(const GameObject* const caller);
	const DirectX::Mouse::ButtonStateTracker*		GetMouse(const GameObject* const caller);

	void											GetMouseRay(physx::PxVec3& origin, physx::PxVec3& ray) const { origin = m_RayOrigin; ray = m_Ray; }
	physx::PxVec2									GetMousePos() const { return m_Camera.GetMousePos(); }
	physx::PxVec2									GetClientSize() const { return m_GDevice->GetClientSize(); }
	float											GetDeltaTime() const { return m_Timer.DeltaTime(); }

public:
	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

protected:
	virtual void Update(float delta) = 0;
	virtual void SelectDevices() = 0;
	virtual void InitObjects() = 0;
	virtual void LoadObjectsFromFile() = 0;
	virtual bool InitMainWindow();
	
	template<typename GraphicDeviceClass> void SelectDeviceByTemplate();

	void SetCurrScene(CGHScene* scene) { m_CurrScene = scene; }
private:
	void CreatePhysxDevice();
	void CameraMove();
	void BaseUpdate();
	void CalculateFrame();

protected:
	static D3DApp*							m_App;

	cCamera									m_Camera;

	DirectX::Mouse							m_Mouse;
	DirectX::Mouse::ButtonStateTracker		m_MouseTracker;

	DirectX::Keyboard						m_Keyboard;
	DirectX::Keyboard::KeyboardStateTracker	m_KeyboardTracker;
	const GameObject*						m_CurrInputDeviceHoldObject;

	physx::PxVec3							m_RayOrigin;
	physx::PxVec3							m_Ray;

	HINSTANCE	m_hAppInst = nullptr;
	HWND		m_hMainWnd = nullptr;
	bool		m_AppPaused = false;
	bool		m_Resizing = false;
	bool		m_Minimized = false;
	bool		m_Maximized = false;

	std::unique_ptr<IGraphicDevice>	m_GDevice;
	PhysX4_1*						m_PXDevice;

private:
	CGHScene*						m_CurrScene;
	GameTimer						m_Timer;
};

template<typename GraphicDeviceClass>
inline void D3DApp::SelectDeviceByTemplate()
{
	assert(!m_GDevice.get());
	assert(!m_PXDevice);

	GraphicDeviceClass* gdeviceInstance = new GraphicDeviceClass();
	std::unique_ptr<IGraphicDevice> temp1(static_cast<IGraphicDevice*>(gdeviceInstance));
	m_GDevice = move(temp1);

	CreatePhysxDevice();
}
