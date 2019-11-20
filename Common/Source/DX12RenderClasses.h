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

	CGH::MAT16	matTransform;
	UINT		diffuseMapIndex = 0;
	int			normalMapIndex= 0;
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

template <typename T>
class UploadBuffer
{
public:
	UploadBuffer(ID3D12Device* device, UINT elementCount, bool isConstantBuffer)
	{
		m_ElementByteSize = sizeof(T);
		m_IsConstantBuffer = isConstantBuffer;

		if (isConstantBuffer)
		{
			m_ElementByteSize = (sizeof(T) + 255) & ~255;
		}

		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(m_ElementByteSize * elementCount),
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
			IID_PPV_ARGS(m_UploadBuffer.GetAddressOf())));

		ThrowIfFailed(m_UploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_MappedData)));
	}

	UploadBuffer(const UploadBuffer& rhs) = delete;
	UploadBuffer& operator=(const UploadBuffer& rhs) = delete;

	~UploadBuffer()
	{
		if (m_UploadBuffer)
		{
			m_UploadBuffer->Unmap(0, nullptr);
		}

		m_MappedData = nullptr;
	}

	ID3D12Resource* Resource()const
	{
		return m_UploadBuffer.Get();
	}

	void CopyData(int elementIndex, const T& data)
	{
		memcpy(&m_MappedData[elementIndex * m_ElementByteSize], &data, sizeof(T));
	}

	void CopyData(int numElement,int offsetIndex, const T& data)
	{
		memcpy(&m_MappedData[offsetIndex * m_ElementByteSize], &data, sizeof(T)* numElement);
	}

private:
	ComPtr<ID3D12Resource> m_UploadBuffer;
	BYTE* m_MappedData = nullptr;

	UINT m_ElementByteSize = 0;
	bool m_IsConstantBuffer = false;
};


class RenderComponent :public IComponent
{
public:
	void SetPrimitiveType(D3D12_PRIMITIVE_TOPOLOGY type) { m_primitiveType = type; }
	void SetGeometry(const MeshObject* geometry, std::string submeshName);
	void SetRenderOK(bool value) { m_isRenderOK = value; }

private:
	void Update();
	void Render(ID3D12GraphicsCommandList* cmdList);
	void SetUploadBufferSize(UINT numInstance);

private:
	UINT m_currFrameCBIndex = 0;
	UINT m_currBufferSize = 4;
	int m_numFrameDirty = 1;

	bool m_isRenderOK = true;
	UINT m_renderInstanceCount = 0;

	const MeshObject* m_geo = nullptr;
	D3D12_PRIMITIVE_TOPOLOGY m_primitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	UINT m_indexCount = 0;
	UINT m_startIndexLocation = 0;
	UINT m_baseVertexLocation = 0;
};