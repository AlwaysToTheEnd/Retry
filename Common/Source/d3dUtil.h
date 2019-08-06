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