#pragma once
#include "ISoundDevice.h"
#include <Audio.h>
#include <memory>

class DXTKAudio final : public ISoundDevice
{
private:
	friend class D3DApp;
	friend class CGHScene;

	struct SoundInstanceByTime
	{
		SoundInstanceByTime()
			: instance(nullptr)
		{

		}

		std::unique_ptr<DirectX::SoundEffectInstance>	instance;
		std::unique_ptr<DirectX::AudioEmitter>			emitter;
		float											time = 0;
	};

public: 
	DXTKAudio() = default;
	virtual ~DXTKAudio() = default;

	virtual void SelectSound(int id, const std::wstring& fileName) override;
	virtual	void Play(int id, unsigned int time, bool loop) override;
	virtual void SetVolume(int id, float vol) override;
	virtual void SetSoundPosition(int id, const physx::PxVec3& pos, const physx::PxVec3& up, float dt) override;
	virtual void SetListener(const physx::PxVec3& pos, float distance) override;

	virtual const std::map<std::wstring, CGH::SoundInfo>& GetStockedSoundList() override;

private: // Used from D3DApp
	virtual bool Init() override;
	virtual void Update() override;

private:
	virtual void RegisterDeviceObject(CGHScene& scene, DeviceObject* gameObject) override {};
	virtual void UnRegisterDeviceObject(CGHScene& scene, DeviceObject* gameObject) override {};

private:
	void LoadStockedSounds(); //Only load filepath
	void CreateSoundEffect(const std::wstring& filePath);

private:
	std::map<std::wstring, CGH::SoundInfo>							m_StockedSoundList;
	std::map<std::wstring, std::unique_ptr<DirectX::SoundEffect>>	m_SoundEffects;
	std::unique_ptr<DirectX::AudioEngine>							m_Engine;
	std::unique_ptr<DirectX::AudioListener>							m_Listener;

	std::map<int, SoundInstanceByTime>								m_Sounds;
};