#include "GraphicBase.h"
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

size_t MeshObject::GetTotalVertexNum() const
{
	size_t result = 0;

	for (auto& it : subs)
	{
		result += it.second.numVertex;
	}

	return result;
}

size_t MeshObject::GetStartVertexOffset() const
{
	size_t result = -1;

	for (auto& it : subs)
	{
		if (result > it.second.vertexOffset)
		{
			result = it.second.vertexOffset;
		}
	}

	return result;
}

size_t MeshObject::GetTotalIndexNum() const
{
	size_t result = 0;

	for (auto& it : subs)
	{
		result += it.second.numIndex;
	}

	return result;
}

size_t MeshObject::GetStartIndexOffset() const
{
	size_t result = -1;

	for (auto& it : subs)
	{
		if (result > it.second.indexOffset)
		{
			result = it.second.indexOffset;
		}
	}

	return result;
}
