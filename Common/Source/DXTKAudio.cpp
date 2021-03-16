#include "DXTKAudio.h"
#include "d3dUtil.h"
#include "d3dApp.h"
#include <Audio.h>
#include <unordered_map>

using namespace DirectX;

void DXTKAudio::PlaySoundFromStock(int id, std::string name, unsigned int time, bool loop)
{
}

void DXTKAudio::PlaySoundFromPath(int id, std::wstring path, unsigned int time, bool loop)
{
	GetFileNameFromPath(path, );
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
	if (!m_Engine->Update())
	{
		//#TODO error.
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
