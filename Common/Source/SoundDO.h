#pragma once
#include <string>
#include <map>
#include "BaseClass.h"
#include "DeviceObject.h"

class ISoundDevice;

class DOSound : public DeviceObject
{
public:
	DOSound(CGHScene& scene, GameObject* parent, const char* typeName)
		: DeviceObject(scene, parent, typeName)
		, m_IsLoop(false)
	{

	}
	virtual	~DOSound() = default;

	void	Play(float time, bool loop);
	void	SelectSound(std::wstring nameORpath);
	void	SetListener(bool value);
	void	SetVolume(float vol);

	const std::map<std::wstring, CGH::SoundInfo>& GetStockedSoundLists() const;

private:
	virtual void Init(IGraphicDevice*, ISoundDevice* SoundDevice, PhysX4_1*);
	virtual void Update(float delta);

private:
	static ISoundDevice*	m_SoundDevice;
	bool					m_IsLoop;
};