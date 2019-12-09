#pragma once

#include <wrl.h>
#include <d3d12.h>
#include "d3dUtil.h"
#include "d3dx12.h"
#include "IComponent.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
class MeshObject;

struct ObjectConstants
{
	CGH::MAT16	world;
	CGH::MAT16	texTransform;
	UINT		materialIndex = 0;
	UINT		pad1;
	UINT		pad2;
	UINT		pad3;
};

struct Material
{
	XMFLOAT4	diffuseAlbedo = { 1,1,1,1 };
	XMFLOAT3	fresnel0 = { 0.01f,0.01f,0.01f };
	float		roughness = 0.25f;

	int			diffuseMapIndex = -1;
	int			normalMapIndex = -1;
	UINT		materialPad1;
	UINT		materialPad2;
};

struct Light
{
	XMFLOAT3 strength = { 0.5f,0.5f,0.5f };
	float falloffStart = 1.0f;
	XMFLOAT3 direction = { 0,-1.0f,0 };
	float falloffEnd = 10.0f;
	XMFLOAT3 position = { 0,0,0 };
	float spotPower = 64.0f;
};

struct PassConstants
{
	CGH::MAT16 view;
	CGH::MAT16 invView;
	CGH::MAT16 proj;
	CGH::MAT16 invProj;
	CGH::MAT16 viewProj;
	CGH::MAT16 invViewProj;
	CGH::MAT16 rightViewProj;
	CGH::MAT16 shadowMapMatrix;
	XMFLOAT3 eyePosW = { 0.0f, 0.0f, 0.0f };
	float cbPerObjectPad1 = 0.0f;
	XMFLOAT2 renderTargetSize = { 0.0f, 0.0f };
	XMFLOAT2 invRenderTargetSize = { 0.0f, 0.0f };

	XMFLOAT4 ambientLight = { 0.0f, 0.0f, 0.0f, 1.0f };

	//Light Lights[16];
};