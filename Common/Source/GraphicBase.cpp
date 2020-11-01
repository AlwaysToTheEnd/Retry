#include "GraphicBase.h"
std::vector<std::wstring> RenderFont::fontNames;

unsigned int RenderFont::GetFontIndex(const std::wstring& fontName)
{
	for (size_t i = 0; i < fontNames.size(); i++)
	{
		if (fontNames[i] == fontName)
		{
			return CGH::SizeTTransUINT(i);
		}
	}

	assert(false && (fontName + L" font was not input").c_str());
	return 0;
}

unsigned int MeshObject::GetTotalVertexNum() const
{
	unsigned int result = 0;

	for (auto& it : subs)
	{
		result += it.second.numVertex;
	}

	return result;
}

unsigned int MeshObject::GetStartVertexOffset() const
{
	unsigned int result = -1;

	for (auto& it : subs)
	{
		if (result > it.second.vertexOffset)
		{
			result = it.second.vertexOffset;
		}
	}

	return result;
}

unsigned int MeshObject::GetTotalIndexNum() const
{
	unsigned int result = 0;

	for (auto& it : subs)
	{
		result += it.second.numIndex;
	}

	return result;
}

unsigned int MeshObject::GetStartIndexOffset() const
{
	unsigned int result = -1;

	for (auto& it : subs)
	{
		if (result > it.second.indexOffset)
		{
			result = it.second.indexOffset;
		}
	}

	return result;
}
