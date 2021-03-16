#include "SoundObject.h"
#include "ISoundDevice.h"

void SoundObject::Init(IGraphicDevice*, ISoundDevice* SoundDevice, PhysX4_1*)
{
	m_SoundDevice = SoundDevice;
}

