#pragma once
#include <unordered_map>
#include <string>

class InputTN
{
public:
	static const std::string& Get(const std::string& purpose);

private:
	static std::unordered_map<std::string, std::string> textures;
};