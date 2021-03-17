#pragma once
#include <map>
#include <vector>
#include <string>
#include "BaseClass.h"
#include "IDeviceObjectRegistration.h"

class D3DApp;

class ISoundDevice : public IDeviceObjectRegistration
{
public:
	friend class D3DApp;
	friend class CGHScene;
public:
	ISoundDevice() = default;
	virtual ~ISoundDevice() = default;

	virtual	void PlaySoundFromStock(int id, const std::wstring& name, unsigned int time, bool loop) = 0;
	virtual	void PlaySoundFromPath(int id, const std::wstring& path, unsigned int time, bool loop) = 0;
	virtual void SetSoundPosition(float x, float y, float z) = 0;
	virtual void SetListener(float x, float y, float z) = 0;
	virtual void AddEmitter() = 0;
	virtual void ControlEmitter() = 0;

	virtual const std::map<std::wstring, CGH::SoundInfo>& GetStockedSoundList() = 0;

protected:
	virtual bool Init() = 0;
	virtual void Update() = 0;

protected:
};

