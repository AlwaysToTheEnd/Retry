#pragma once
#include <unordered_map>
#include <string>
#include <vector>

static class EnumNameRegister
{
public:
	void AddEnum(const std::wstring& enumName, const std::wstring& elements);

private:
	std::unordered_map<std::wstring, std::vector<std::wstring>> m_EnumStrings;
} s_EnumNameRegister;
