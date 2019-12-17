#include "DX12RenderClasses.h"

std::vector<std::wstring> RenderFont::fontNames;

unsigned int RenderFont::GetFontIndex(const std::wstring& fontName)
{
	for (size_t i = 0; i < fontNames.size(); i++)
	{
		if (fontNames[i] == fontName)
		{
			return i;
		}
	}

	assert(false && (fontName + L" font was not input").c_str());
	return 0;
}
