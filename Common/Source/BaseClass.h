#pragma once
#include <DirectXMath.h>
#include <foundation/PxMat44.h>

struct ENUM_ELEMENT
{
	int value = 0;
	std::wstring elementName;
};

namespace CGH
{
	enum class DATA_TYPE
	{
		TYPE_BOOL,
		TYPE_FLOAT,
		TYPE_INT,
		TYPE_UINT,
	};

	struct UnionData
	{
		DATA_TYPE type;
		union
		{
			bool	_b;
			float	_f;
			int		_i;
			unsigned int _u;
		};
	};

	enum MESH_TYPE
	{
		MESH_NORMAL,
		MESH_SKINED,
		MESH_NONE
	};
}