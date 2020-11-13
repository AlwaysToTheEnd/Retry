#pragma once
#include <foundation/PxMat44.h>
#include <foundation/PxVec2.h>
#include <foundation/PxVec3.h>
#include <string>
#include <intsafe.h>
#include "InputTextureNameList.h" 

struct ENUM_ELEMENT
{
	int value = 0;
	std::wstring elementName;
};

namespace CGH
{
	inline unsigned int SizeTTransUINT(size_t size)
	{
		assert(UINT_MAX > size);
		return static_cast<unsigned int>(size);
	}

	inline int SizeTTransINT(size_t size)
	{
		assert(INT_MAX > size);
		return static_cast<int>(size);
	}

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

		struct InOutOption
		{
			float nonClickedHeldTime = 0.15f;

		}inOut;

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