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
		, m_SoundDevice(nullptr)
		, m_IsLoop(false)
	{

	}
	virtual	~DOSound() = default;

	void	PlaySoundFromStock(std::wstring name, unsigned int time, bool loop);
	void	PlaySoundFromPath(std::wstring path, unsigned int time, bool loop);

	const std::map<std::wstring, CGH::SoundInfo>& GetStockedSoundLists() const;

private:
	virtual void Init(IGraphicDevice*, ISoundDevice* SoundDevice, PhysX4_1*);
	virtual void Update(float delta) {};

private:
	ISoundDevice*	m_SoundDevice;
	bool			m_IsLoop;
};