#include "InputTextureNameList.h"
#include <assert.h>
std::unordered_map<std::string, std::string> InputTN::textures =
{
	{"AniTreeCreatorWorkPanel","closeButton.png"},
	{"AniTreeCreatorWorkPanel_AddButton","closeButton.png"},
	{"AniNodeVisualPanel","closeButton.png"},
	{"AniTreeArrowVisual","closeButton.png"},
	{"AniTreeArrowArttributePanel","closeButton.png"},
	{"AniTreeArrowArttributePanel_AddButton","closeButton.png"},
};


const std::string& InputTN::Get(const std::string& purpose)
{
	auto iter = textures.find(purpose);
	assert(iter != textures.end());

	return iter->second;
}
