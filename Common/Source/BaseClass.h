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
			int				panelComponentsInterval = 5;
		}ui;

		struct GraphicOption
		{
			float				onedayTime = 24.0f;
			bool				dirLightOn = true;
			unsigned int		samplerIndex = 2;
			bool				shadowMap = true;
			bool				cullMode = true;
			bool				wireFrameMode = false;
			physx::PxVec3		dirLight = { 0,-1,0 };
			float				dirLightPower =1;
			float				fovAngleY= 0.785398163f;
			float				fovAngleX= 0;
			float				aspectRatio = 1;
			float				perspectiveNearZ = 1.0f;
			float				perspectiveFarZ = 1000.0f;

		}graphic;

		struct PhysicsOption
		{
			bool drawCollisionShapeWireFrame = false;

		}physics;

	} GO;
}