#pragma once
#include <array>
#include "../Common/Source/PhysX4_1.h"
#include "../Common/Source/GraphicDX12.h"
#include "../Common/Source/d3dApp.h"

//using typedef to select core devices.
typedef GraphicDX12	UsingGraphicDevice;
typedef PhysX4_1	UsingPhsicsDevice;

class Step2 final : public D3DApp
{
public:
	Step2(HINSTANCE hInstance);
	Step2(const Step2& rhs) = delete;
	Step2& operator=(const Step2& rhs) = delete;
	virtual ~Step2();

	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) override;

private:
	virtual void Update() override;
	virtual void SelectDevices() override;

private:
	virtual void InitObjects() override;

private:
};