#pragma once
#include <DirectXMath.h>

struct Vertex
{
	Vertex(DirectX::XMFLOAT3 _pos, DirectX::XMFLOAT3 _normal, DirectX::XMFLOAT2 _uv)
	{
		position = _pos;
		normal = normal;
		uv = _uv;
	}

	DirectX::XMFLOAT3 position = { 0,0,0 };
	DirectX::XMFLOAT3 normal = { 0,0,-1 };
	DirectX::XMFLOAT2 uv = { 0,0 };
};


struct SkinnedVertex
{
	DirectX::XMFLOAT3	position;
	DirectX::XMFLOAT3	normal;
	DirectX::XMFLOAT2	uv;
	DirectX::XMFLOAT3	boneWeights;
	unsigned char		boneIndices[4];
};