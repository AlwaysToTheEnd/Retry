#include "InputTextureNameList.h"
#include <assert.h>
std::unordered_map<std::string, std::string> InputTN::textures =
{
	{"AniTreeCreatorWorkPanel_AddButton",		"closeButton.png"},
	{"AniNodeVisualPanel_Delete",				"closeButton.png"},
	{"AniTreeArrowVisual",						"AniTreeArrow.png"},
	{"AniTreeArrowArttributePanel_AddButton",	"closeButton.png"},
	{"AniTreeArrowArttributePanel_DeleteButton","closeButton.png"},
};


const std::string& InputTN::Get(const std::string& purpose)
{
	auto iter = textures.find(purpose);
	assert(iter != textures.end());

	return iter->second;
}
