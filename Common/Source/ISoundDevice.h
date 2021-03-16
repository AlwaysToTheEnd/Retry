#pragma once
#include <map>
#include <vector>
#include <string>
#include "IDeviceObjectRegistration.h"

class D3DApp;

namespace CGH
{
	struct SoundInfo
	{
		unsigned int	timeRate = 0;
		int				format = 0;
		std::wstring	filePath;
	};
}

class ISoundDevice : public IDeviceObjectRegistration
{
public:
	friend class D3DApp;
	friend class CGHScene;
public:
	ISoundDevice() = default;
	virtual ~ISoundDevice() = default;

	virtual	void PlaySoundFromStock(int id, std::string name, unsigned int time, bool loop) = 0;
	virtual	void PlaySoundFromPath(int id, std::wstring path, unsigned int time, bool loop) = 0;

	virtual const std::map<std::string, CGH::SoundInfo>& GetStockedSoundList() = 0;

protected:
	virtual bool Init() = 0;
	virtual void Update() = 0;

protected:
};

