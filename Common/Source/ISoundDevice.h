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

	virtual void SelectSound(int id, const std::wstring& fileName) = 0;
	virtual	void Play(int id, unsigned int time, bool loop) = 0;
	virtual void SetVolume(int id, float vol) = 0;
	virtual void SetSoundPosition(int id, const physx::PxVec3& pos, const physx::PxVec3& up, float dt) = 0;
	virtual void SetListener(const physx::PxVec3& pos, float distance) = 0;

	virtual const std::map<std::wstring, CGH::SoundInfo>& GetStockedSoundList() = 0;

protected:
	virtual bool Init() = 0;
	virtual void Update() = 0;

protected:
};

