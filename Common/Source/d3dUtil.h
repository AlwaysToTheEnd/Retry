#pragma once

#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#define WIN32_LEAN_AND_MEAN         
#include <windows.h>

#include <memory>
#include <tchar.h>
#include <assert.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <comdef.h>
#include "BaseClass.h"
#include <io.h>

class DxException
{
public:
	DxException() = default;
	DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber);

	std::wstring ToString()const;

	HRESULT ErrorCode = S_OK;
	std::wstring FunctionName;
	std::wstring Filename;
	int LineNumber = -1;
};

inline std::wstring AnsiToWString(const std::string& str)
{
	WCHAR buffer[512];
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
	return std::wstring(buffer);
}

#define ThrowIfFailed(x)                                              \
{                                                                     \
    HRESULT hr__ = (x);                                               \
    std::wstring wfn = AnsiToWString(__FILE__);                       \
    if(FAILED(hr__)) { throw DxException(hr__, L#x, wfn, __LINE__); } \
}

#define Throw_And_SetName(descFunc, resource) 									\
{																				\
	HRESULT hr__ = (descFunc);												    \
	std::wstring wfn = AnsiToWString(__FILE__);                    				\
	if(FAILED(hr__)) { throw DxException(hr__, L#descFunc, wfn, __LINE__); }	\
	resource->SetName(std::wstring(wfn+L"Line"+ std::to_wstring(__LINE__)+std::wstring(L#resource)).c_str());	\
}													

inline void DebugTrace(_In_z_ _Printf_format_string_ const char* format, ...) noexcept
{
#ifdef _DEBUG
	va_list args;
	va_start(args, format);

	char buff[1024] = {};
	vsprintf_s(buff, format, args);
	OutputDebugStringA(buff);
	va_end(args);
#else
	UNREFERENCED_PARAMETER(format);
#endif
}

inline std::wstring GetFolderNameFromFilePath(const std::wstring& filePath)
{
	std::wstring result;

	size_t lastOneIndex = filePath.find_last_of(L'\\');
	size_t lastTwoIndex = filePath.find_last_of(L'\\', 1);

	if (lastTwoIndex == std::wstring::npos)
	{
		result.insert(0, filePath, lastOneIndex - 1);
	}
	else
	{
		result.insert(lastTwoIndex + 1, filePath, lastOneIndex - lastTwoIndex - 1);
	}

	return result;
}

inline void SearchAllFolderFromFolder(const std::wstring& folderPath, std::vector<std::string>& out)
{
	WIN32_FIND_DATA fd;
	HANDLE handle = 0;
	int result = 1;
	handle = FindFirstFile((folderPath + L"\\*").c_str(), &fd);

	if (handle == INVALID_HANDLE_VALUE)
	{
		FindClose(handle);
		return;
	}

	std::wstring subFolderPath = folderPath + L"/";

	while (result)
	{
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (fd.cFileName[0] != '.')
			{
				std::wstring temp(fd.cFileName);
				out.push_back(std::string(temp.begin(), temp.end()));
			}
		}

		result = FindNextFile(handle, &fd);
	}

	FindClose(handle);
}

inline void SearchAllFolderFromFolder(const std::wstring& folderPath, std::vector<std::wstring>& out)
{
	WIN32_FIND_DATA fd;
	HANDLE handle = 0;
	int result = 1;
	handle = FindFirstFile((folderPath + L"\\*").c_str(), &fd);

	if (handle == INVALID_HANDLE_VALUE)
	{
		FindClose(handle);
		return;
	}

	std::wstring subFolderPath = folderPath + L"/";

	while (result)
	{
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (fd.cFileName[0] != '.')
			{
				out.push_back(fd.cFileName);
			}
		}

		result = FindNextFile(handle, &fd);
	}

	FindClose(handle);
}

inline void SearchAllFileFromFolder(const std::wstring& folderPath, bool searchSubfolder, std::vector<std::string>& out)
{
	WIN32_FIND_DATA fd;
	HANDLE handle = 0;
	int result = 1;
	handle = FindFirstFile((folderPath + L"\\*").c_str(), &fd);

	if (handle == INVALID_HANDLE_VALUE)
	{
		FindClose(handle);
		return;
	}

	std::wstring subFolderPath = folderPath + L"/";
	std::vector<std::wstring> subFolders;

	while (result)
	{
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (searchSubfolder)
			{
				if (fd.cFileName[0] != '.')
				{
					subFolders.push_back(fd.cFileName);
				}
			}
		}
		else
		{
			std::wstring filePath = subFolderPath + fd.cFileName;
			std::string temp;
			temp.insert(temp.end(), filePath.begin(), filePath.end());
			out.push_back(temp);
		}

		result = FindNextFile(handle, &fd);
	}

	if (searchSubfolder)
	{
		for (auto& it : subFolders)
		{
			SearchAllFileFromFolder(subFolderPath + it, searchSubfolder, out);
		}
	}

	FindClose(handle);
}

inline void SearchAllFileFromFolder(const std::wstring& folderPath, bool searchSubfolder, std::vector<std::wstring>& out)
{
	WIN32_FIND_DATA fd;
	HANDLE handle = 0;
	int result = 1;
	handle = FindFirstFile((folderPath + L"\\*").c_str(), &fd);

	if (handle == INVALID_HANDLE_VALUE)
	{
		FindClose(handle);
		return;
	}

	std::wstring subFolderPath = folderPath + L"/";
	std::vector<std::wstring> subFolders;

	while (result)
	{
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (searchSubfolder)
			{
				if (fd.cFileName[0] != '.')
				{
					subFolders.push_back(fd.cFileName);
				}
			}
		}
		else
		{
			std::wstring filePath = subFolderPath + fd.cFileName;
			out.push_back(filePath);
		}

		result = FindNextFile(handle, &fd);
	}

	if (searchSubfolder)
	{
		for (auto& it : subFolders)
		{
			SearchAllFileFromFolder(subFolderPath + it, searchSubfolder, out);
		}
	}

	FindClose(handle);
}

inline std::string GetFileNameFromPath(const std::string& path, std::string& extension, bool includeExetension = true)
{
	std::string temp;
	for (auto& it : path)
	{
		if (it == '/' || it == '\\')
		{
			temp.clear();
		}
		else
		{
			temp += it;
		}
	}

	for (auto& it : temp)
	{
		if (it == '.')
		{
			extension.clear();
		}
		else
		{
			extension += it;
		}
	}

	if (!includeExetension)
	{
		for (size_t i = 0; i < extension.size() + 1; i++)
		{
			temp.pop_back();
		}
	}

	return temp;
}

inline std::wstring GetFileNameFromPath(const std::wstring& path, std::wstring& extension, bool includeExetension = true)
{
	std::wstring temp;
	for (auto& it : path)
	{
		if (it == '/')
		{
			temp.clear();
		}
		else
		{
			temp += it;
		}
	}

	for (auto& it : temp)
	{
		if (it == '.')
		{
			extension.clear();
		}
		else
		{
			extension += it;
		}
	}

	if (!includeExetension)
	{
		for (size_t i = 0; i < extension.size() + 1; i++)
		{
			temp.pop_back();
		}
	}

	return temp;
}

static std::vector<std::string> surpportedTextureExtension
{
	"jpg", "dds", "bmp", "png", "tga"
};

static std::vector<std::string> surpportedMeshExtension
{
	"X"
};

static std::vector<std::string> surpportedSoundExtension
{
	"wav"
};

enum class EXTENSIONTYPE
{
	EXE_TEXTURE,
	EXE_MESH,
	EXE_WAVE,
	EXE_NONE_SUPPORT
};


inline EXTENSIONTYPE CheckFileExtension(const std::string& extension)
{
	for (auto& it : surpportedTextureExtension)
	{
		if (extension == it)
		{
			return EXTENSIONTYPE::EXE_TEXTURE;
		}
	}

	for (auto& it : surpportedMeshExtension)
	{
		if (extension == it)
		{
			return EXTENSIONTYPE::EXE_MESH;
		}
	}

	for (auto& it : surpportedSoundExtension)
	{
		if (extension == it)
		{
			return EXTENSIONTYPE::EXE_WAVE;
		}
	}

	return EXTENSIONTYPE::EXE_NONE_SUPPORT;
}

inline EXTENSIONTYPE CheckFileExtension(const std::wstring& extension)
{
	std::string temp;
	temp.assign(extension.begin(), extension.end());
	
	return CheckFileExtension(temp);
}