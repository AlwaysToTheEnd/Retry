#pragma once
#include "ISoundDevice.h"
#include <memory>

namespace DirectX
{
	class AudioEngine;
	class SoundEffect;
	class SoundEffectInstance;
	class SoundStreamInstance;
	class WaveBank;
}

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

		std::unique_ptr<DirectX::SoundEffectInstance> instance;
		unsigned int time = 0;
	};

public:
	DXTKAudio() = default;
	virtual ~DXTKAudio() = default;

	virtual	void PlaySoundFromStock(int id, const std::string& name, unsigned int time, bool loop) override;
	virtual	void PlaySoundFromPath(int id, const std::wstring& path, unsigned int time, bool loop) override;
	virtual void SetSoundPosition(float x, float y, float z) override;
	virtual void SetListener(float x, float y, float z) override;
	virtual void AddEmitter() override;
	virtual void ControlEmitter() override;

	virtual const std::map<std::string, CGH::SoundInfo>& GetStockedSoundList() override;

private: // Used from D3DApp
	virtual bool Init() override;
	virtual void Update() override;

private:
	void LoadStockedSounds(); //Only load filepath
	void PlayedObjectRelease(int id);

private:
	std::map<std::string, CGH::SoundInfo>							m_StockedSoundList;
	std::map<std::wstring, std::unique_ptr<DirectX::SoundEffect>>	m_SoundEffects;
	std::unique_ptr<DirectX::AudioEngine>							m_Engine;
	std::unique_ptr<DirectX::WaveBank>								m_WaveBank;

	std::map<int, SoundInstanceByTime>								m_Sounds;
};