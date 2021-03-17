#include "SoundDO.h"
#include "ISoundDevice.h"

void DOSound::PlaySoundFromStock(std::wstring name, unsigned int time, bool loop)
{
	m_SoundDevice->PlaySoundFromStock(GetDeviceOBID(), name, time, loop);

}

void DOSound::PlaySoundFromPath(std::wstring path, unsigned int time, bool loop)
{
	m_SoundDevice->PlaySoundFromPath(GetDeviceOBID(), path, time, loop);
}

const std::map<std::wstring, CGH::SoundInfo>& DOSound::GetStockedSoundLists() const
{
	return m_SoundDevice->GetStockedSoundList();
}


void DOSound::Init(IGraphicDevice*, ISoundDevice* SoundDevice, PhysX4_1*)
{
	m_SoundDevice = SoundDevice;
}

