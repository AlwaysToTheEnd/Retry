#include "DXTKAudio.h"

#include <Audio.h>
#include <assert.h>
#include <unordered_map>
#include "d3dUtil.h"
#include "d3dApp.h"

using namespace DirectX;

void DXTKAudio::SelectSound(int id, const std::wstring& fileName)
{
	auto iter = m_StockedSoundList.find(fileName);

	if (iter == m_StockedSoundList.end()) // To check this filename is filePath.
	{
		std::wstring extension;
		std::wstring path = fileName;

		if (m_SoundEffects.find(path) == m_SoundEffects.end())
		{
			CreateSoundEffect(path);
		}

		iter = m_StockedSoundList.find(GetFileNameFromPath(fileName, extension));
	}
	else
	{
		if (m_SoundEffects[iter->second.filePath] == nullptr)
		{
			CreateSoundEffect(iter->second.filePath);
		}
	}

	auto currSoundEffect = m_SoundEffects[iter->second.filePath].get();

	if (m_Sounds[id].instance != nullptr)
	{
		m_Sounds[id].instance->Stop();
	}

	m_Sounds[id].instance = nullptr;
	m_Sounds[id].instance = std::move(currSoundEffect->CreateInstance(SoundEffectInstance_Use3D));
}

void DXTKAudio::Play(int id, unsigned int time, bool loop)
{
	if (m_Sounds[id].instance != nullptr)
	{
		m_Sounds[id].instance->Stop();
		m_Sounds[id].instance->Play(loop);
		m_Sounds[id].time = time;
	}
}

void DXTKAudio::SetVolume(int id, float vol)
{
	m_Sounds[id].instance->SetVolume(vol);
}

void DXTKAudio::SetSoundPosition(int id, const physx::PxVec3& pos, const physx::PxVec3& up, float dt)
{
	auto iter = m_Sounds.find(id);

	if (iter != m_Sounds.end())
	{
		if (iter->second.emitter==nullptr)
		{
			iter->second.emitter = std::move(std::make_unique<DirectX::AudioEmitter>());
		}

		DirectX::XMVECTOR position = DirectX::XMVectorSet(pos.x, pos.y, pos.z, 0);
		DirectX::XMVECTOR upDir = DirectX::XMVectorSet(up.x, up.y, up.z, 0);

		iter->second.emitter->Update(position, upDir, dt);

		if (iter->second.instance)
		{
			iter->second.instance->Apply3D(*m_Listener, *iter->second.emitter);
		}
	}
	else
	{
		assert(false);
	}
}

void DXTKAudio::SetListener(const physx::PxVec3& pos, float distance)
{
	m_Listener->Update(DirectX::XMVectorSet(pos.x, pos.y, pos.z, 1), DirectX::XMVectorSet(0, 1, 0, 0), distance);
}

const std::map<std::wstring, CGH::SoundInfo>& DXTKAudio::GetStockedSoundList()
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

	m_Listener = std::make_unique<DirectX::AudioListener>();

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
		if (it.second.instance != nullptr)
		{
			if (it.second.time <= 0)
			{
				if (!it.second.instance->IsLooped())
				{
					it.second.instance->Stop();
				}

				it.second.time = 0;
			}

			it.second.time -= delta;
		}
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
					CGH::SoundInfo temp;

					m_SoundEffects[it2] = std::make_unique<SoundEffect>(m_Engine.get(), it2.c_str());
					temp.timeRate = m_SoundEffects[it2]->GetSampleDurationMS();
					temp.filePath = it2;

					m_StockedSoundList.insert({ fileName, temp });
				}
			}

			files.clear();
		}
	}
}

void DXTKAudio::CreateSoundEffect(const std::wstring& filePath)
{
	std::wstring extension;
	std::wstring name = GetFileNameFromPath(filePath, extension);

	if (CheckFileExtension(extension) != EXTENSIONTYPE::EXE_WAVE)
	{
		assert(false && "this file extension is not wav");
	}

	if (m_SoundEffects[filePath] == nullptr)
	{
		m_SoundEffects[filePath] = std::make_unique<SoundEffect>(m_Engine.get(), filePath.c_str());

		CGH::SoundInfo soundInfo;
		soundInfo.filePath = filePath;
		soundInfo.timeRate = m_SoundEffects[filePath]->GetSampleDurationMS();

		if (m_StockedSoundList.find(name) == m_StockedSoundList.end())
		{
			m_StockedSoundList[name] = soundInfo;
		}
	}
}