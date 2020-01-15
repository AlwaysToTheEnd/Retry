#pragma once
#include <foundation/PxMat44.h>
#include <foundation/PxVec2.h>
#include <foundation/PxVec3.h>
#include <string>
#include "InputTextureNameList.h"

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

	static struct GlobalOptions
	{
		struct ClientOption
		{
			float defaultCameraDim = 50.0f;

		}client;

		struct UIOption
		{
			physx::PxVec4	panelTitleColor = { 0.1f, 0.1f, 0.1f, 1.0f };
			int				panelTitleHeight = 20;
			int				panelTitleTextHeight = 15;
			int				panelCloseButtonHalfSize = panelTitleHeight / 2;
			int				panelComponentsInterval = 3;
		}ui;

		struct GraphicOption
		{
			physx::PxVec3	dirLight = { 0,-1,0 };
			physx::PxVec3	dirLightPower = { 1,1,1 };
			unsigned int	samplerIndex = 0;
			bool			shadowMap = true;
			bool			dirLightOn = true;
			bool			cullMode = true;
			bool			wireFrameMode = false;

		}graphic;

		struct PhysicsOption
		{
			bool drawCollisionShapeWireFrame = false;

		}physics;

	} GO;
}