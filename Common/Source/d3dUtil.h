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

void SearchAllFileFromFolder(const std::string& folderPath, bool searchSubfolder, std::vector<std::string>& out)
{
	_finddata_t fd;

	long handle = 0;
	int result = 1;

	handle = _findfirst(folderPath.c_str(), &fd);

	assert(handle != -1 && (folderPath + " dose not exist").c_str());

	std::string subFolderPath = folderPath + "//";
	std::vector<std::string> subFolders;

	while (result != -1)
	{
		if ((fd.attrib & _A_SUBDIR) == 0)
		{
			if (searchSubfolder)
			{
				subFolders.push_back(fd.name);
			}
		}
		else
		{
			out.push_back(subFolderPath + fd.name);
		}

		result = _findnext(handle, &fd);
	}

	if (searchSubfolder)
	{
		for (auto& it : subFolders)
		{
			SearchAllFileFromFolder(subFolderPath + it, searchSubfolder, out);
		}
	}

	_findclose(handle);
}

std::string GetFileNameFromPath(const std::string& path, std::string& extension)
{
	std::string temp;
	for (auto& it : path)
	{
		if (it == '//')
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

	return temp;
}

static std::vector<std::string> surpportedTextureExtension
{
	"jpg", "dds"
};

static std::vector<std::string> surpportedMeshExtension
{
	"X"
};

enum class EXTENSIONTYPE
{
	EXE_TEXTURE,
	EXE_MESH,
	EXE_NONE_SUPPORT
};


EXTENSIONTYPE CheckFileExtension(const std::string& extension)
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

	return EXTENSIONTYPE::EXE_NONE_SUPPORT;
}