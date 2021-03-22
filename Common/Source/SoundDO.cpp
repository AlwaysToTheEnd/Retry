#include "SoundDO.h"
#include "PhysicsDO.h"
#include "ISoundDevice.h"

ISoundDevice* DOSound::m_SoundDevice = nullptr;

void DOSound::Play(float time, bool loop)
{
	m_SoundDevice->Play(GetDeviceOBID(), time, loop);
}

void DOSound::SelectSound(std::wstring nameORpath)
{
	m_SoundDevice->SelectSound(GetDeviceOBID(), nameORpath);
}

void DOSound::SetListener(bool value)
{

}

void DOSound::SetVolume(float vol)
{
	m_SoundDevice->SetVolume(GetDeviceOBID(),vol);
}

const std::map<std::wstring, CGH::SoundInfo>& DOSound::GetStockedSoundLists() const
{
	return m_SoundDevice->GetStockedSoundList();
}


void DOSound::Init(IGraphicDevice*, ISoundDevice* SoundDevice, PhysX4_1*)
{
	if (m_SoundDevice == nullptr)
	{
		m_SoundDevice = SoundDevice;
	}
}

void DOSound::Update(float delta)
{
	auto transformDO = m_Parent->GetComponent<DOTransform>();

	if (transformDO)
	{
		auto transform = transformDO->GetTransform();
		m_SoundDevice->SetSoundPosition(GetDeviceOBID(), transform.p, { 0,1,0 }, 10.0f);
	}
}

