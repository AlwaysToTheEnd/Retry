#pragma once
#include "IGraphicDevice.h"
#include "DeviceObject.h"
#include "CGHScene.h"
#include <Mouse.h>
#include <Keyboard.h>
#include "cCamera.h"
#include "GameTimer.h"

#define GETAPP D3DApp::GetApp()
#define GETMOUSEPOS D3DApp::GetApp()->GetMousePos()
#define GETMOUSEMOUVEDVALUE D3DApp::GetApp()->GetMouseMovedValue()
#define GETKEY D3DApp::GetApp()->GetKeyBoard()
#define PUSHEDESC D3DApp::GetApp()->IsPushedESC()
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

	const DirectX::Keyboard::KeyboardStateTracker&	GetKeyBoard();
	bool											IsPushedESC() { return m_IsPushedESC; }

	void											GetMouseRay(physx::PxVec3& origin, physx::PxVec3& ray) const { origin = m_RayOrigin; ray = m_Ray; }
	bool											IsMouseButtonClickedAndNotThisObject(DirectX::MOUSEBUTTONINDEX buttonIndex, int id);
	bool											IsMouseButtonClicked(DirectX::MOUSEBUTTONINDEX buttonIndex);
	physx::PxVec2									GetMousePos() const { return m_Camera.GetMousePos(); }
	physx::PxVec2									GetMouseMovedValue() const { return m_MouseMovedValue; }
	float											GetMouseHeldTime(DirectX::MOUSEBUTTONINDEX buttonIndex) const { return m_MouseHeldTime[static_cast<UINT>(buttonIndex)]; }
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
	void UpdatePixelFuncFromMouse();
	void UpdateMouseBaseClick(float delta);

protected:
	static D3DApp*							m_App;

	cCamera									m_Camera;

	DirectX::Mouse							m_Mouse;
	DirectX::Mouse::ButtonStateTracker		m_MouseTracker;
	physx::PxVec2							m_MousePrevPos;
	physx::PxVec2							m_MouseMovedValue;
	float									m_MouseHeldTime[static_cast<UINT>(DirectX::MOUSEBUTTONINDEX::COUNT)];
	int										m_PrevPixelFuncIndex;

	bool									m_IsPushedESC;
	DirectX::Keyboard						m_Keyboard;
	DirectX::Keyboard::KeyboardStateTracker	m_KeyboardTracker;

	physx::PxVec3							m_RayOrigin;
	physx::PxVec3							m_Ray;
	std::vector<int>						m_PixelFuncMap;

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
