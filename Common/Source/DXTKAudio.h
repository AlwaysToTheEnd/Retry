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
	friend class D3DApp;
	friend class CGHScene;

public:
	DXTKAudio() = default;
	virtual ~DXTKAudio() = default;

	virtual	void PlaySoundFromStock(int id, std::string name, unsigned int time, bool loop) override;
	virtual	void PlaySoundFromPath(int id, std::wstring path, unsigned int time, bool loop) override;

	virtual const std::map<std::string, CGH::SoundInfo>& GetStockedSoundList() override;

private: // Used from D3DApp
	virtual bool Init() override;
	virtual void Update() override;

private:
	void LoadStockedSounds(); //Only load filepath

private:
	std::map<std::string, CGH::SoundInfo>							m_StockedSoundList;
	std::map<std::string, std::unique_ptr<DirectX::SoundEffect>>	m_SoundEffects;
	std::unique_ptr<DirectX::AudioEngine>							m_Engine;
	std::unique_ptr<DirectX::WaveBank>								m_WaveBank;

	std::map<int, std::unique_ptr<DirectX::SoundEffectInstance>>	m_Sounds
};