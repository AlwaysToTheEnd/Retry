#pragma once
#include <string>
#include "DeviceObject.h"

class ISoundDevice;

class SoundObject : public DeviceObject
{
public:
	SoundObject(CGHScene& scene, GameObject* parent, const char* typeName)
		: DeviceObject(scene, parent, typeName)
		, m_SoundDevice(nullptr)
		, m_IsLoop(false)
	{

	}
	virtual							~SoundObject() = default;

	void							PlaySoundFromStock(std::string name, unsigned int time, bool loop);
	void							PlaySoundFromPath(std::string name, unsigned int time, bool loop);

	const std::vector<std::string>& GetStockedSoundLists() const;

private:
	virtual void Init(IGraphicDevice*, ISoundDevice* SoundDevice, PhysX4_1*);
	virtual void Update(float delta) {};

private:
	ISoundDevice*	m_SoundDevice;
	bool			m_IsLoop;
};