#include "DXTKAudio.h"
#include "d3dUtil.h"
#include "d3dApp.h"
#include <Audio.h>
#include <unordered_map>

using namespace DirectX;

void DXTKAudio::PlaySoundFromStock(int id, const std::string& name, unsigned int time, bool loop)
{
	PlayedObjectRelease(id);

}

void DXTKAudio::PlaySoundFromPath(int id, const std::wstring& path, unsigned int time, bool loop)
{
	PlayedObjectRelease(id);

	std::wstring extension;
	std::wstring fileName = GetFileNameFromPath(path, extension);

	if (CheckFileExtension(extension) != EXTENSIONTYPE::EXE_WAVE)
	{
		assert(false && "this file extension is not wav");
	}

	if (m_SoundEffects.find(path) == m_SoundEffects.end())
	{
		auto soundEffect = std::make_unique<SoundEffect>(m_Engine.get(), path.c_str());
		m_SoundEffects[path] = std::move(soundEffect);
		std::string fileNametemp(fileName.begin(), fileName.end());
		CGH::SoundInfo soundInfo;
		soundInfo.filePath = path;
		soundInfo.timeRate = m_SoundEffects[path]->GetSampleDurationMS();
		m_StockedSoundList.insert({ fileNametemp ,soundInfo });
	}

	auto currSoundEffect = m_SoundEffects[path].get();
	
	m_Sounds[id].instance= std::move(currSoundEffect->CreateInstance(SoundEffectInstance_Default));

	m_Sounds[id].instance->Play(loop);
	m_Sounds[id].time = time;
}

void DXTKAudio::SetSoundPosition(float x, float y, float z)
{

}

const std::map<std::string, CGH::SoundInfo>& DXTKAudio::GetStockedSoundList()
{
	return m_StockedSoundList;
}

bool DXTKAudio::Init()
{
	ThrowIfFailed(CoInitializeEx(nullptr, COINIT_MULTITHREADED));

	AUDIO_ENGINE_FLAGS engineFlags = AudioEngine_Default;

#ifdef _DEBUG
	engineFlags = AudioEngine_Debug | AudioEngine_Default;
#endif // DEBUG

	m_Engine = std::make_unique<AudioEngine>(engineFlags);

	LoadStockedSounds();

	return m_Engine != nullptr;
}

void DXTKAudio::Update()
{
	float delta = GETAPP->GetDeltaTime();

	if (!m_Engine->Update())
	{
		//#TODO error.
	}

	for (auto& it : m_Sounds)
	{
		it.second.time -= delta;
	}
}

void DXTKAudio::LoadStockedSounds()
{
	m_StockedSoundList.clear();
	const auto& targetSoundFolders = GETAPP->m_TargetSoundFolders;
	std::vector<std::vector<std::wstring>> folders(targetSoundFolders.size());
	std::vector<std::wstring> files;

	for (size_t i = 0; i < folders.size(); i++)
	{
		SearchAllFolderFromFolder(targetSoundFolders[i], folders[i]);
	}

	for (size_t i = 0; i < folders.size(); i++)
	{
		for (auto& it : folders[i])
		{
			SearchAllFileFromFolder(targetSoundFolders[i] + L"//" + it, true, files);

			for (auto& it2 : files)
			{
				std::wstring extension;
				std::wstring fileName = GetFileNameFromPath(it2, extension);

				if (CheckFileExtension(extension) == EXTENSIONTYPE::EXE_WAVE)
				{
					std::string fileNameTemp;
					CGH::SoundInfo temp;

					fileNameTemp.assign(fileName.begin(), fileName.end());
					temp.filePath = it2;

					m_StockedSoundList[fileNameTemp] = temp;
				}
			}

			files.clear();
		}
	}
}

void DXTKAudio::PlayedObjectRelease(int id)
{
	auto result = m_Sounds.find(id);
	
	if (result != m_Sounds.end())
	{
		result->second.instance->Stop();
		result->second.instance.~unique_ptr();
	}
	else
	{
		SoundInstanceByTime temp;
		m_Sounds.emplace( id, temp );
	}
	
}