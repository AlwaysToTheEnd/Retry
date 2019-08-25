#pragma once

#include <wrl.h>
#include <d3d12.h>
#include "d3dUtil.h"
#include "d3dx12.h"
#include "IComponent.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;

struct ObjectConstants
{
	CGH::MAT16 world;
	CGH::MAT16 TexTransform;
};

struct Material
{
	std::string	name;
	int			matCBIndex = -1;

	XMFLOAT4	diffuseAlbedo = { 1,1,1,1 };
	XMFLOAT3	fresnel0 = { 0.01f,0.01f,0.01f };
	float		roughness = 0.25f;
	CGH::MAT16	matTransform;
	UINT		diffuseMapIndex = 0;
};

struct MaterialConstants
{
	XMFLOAT4	diffuseAlbedo = { 1,1,1,1 };
	XMFLOAT3	fresnel0 = { 0.01f,0.01f,0.01f };
	float		roughness = 0.25f;

	CGH::MAT16	matTransform;
	UINT		diffuseMapIndex = 0;
	UINT		MaterialPad0;
	UINT		MaterialPad1;
	UINT		MaterialPad2;
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

struct SubMeshGeometry
{
	UINT indexCount = 0;
	UINT startIndexLocation = 0;
	UINT baseVertexLocation = 0;
};

struct MeshGeometry
{
	std::string name;

	ComPtr<ID3DBlob> vertexBufferCPU = nullptr;
	ComPtr<ID3DBlob> indexBufferCPU = nullptr;

	ComPtr<ID3D12Resource> vertexBufferGPU = nullptr;
	ComPtr<ID3D12Resource> indexBufferGPU = nullptr;

	ComPtr<ID3D12Resource> vertexUploadBuffer = nullptr;
	ComPtr<ID3D12Resource> indexUploadBuffer = nullptr;

	UINT vertexByteStride = 0;
	UINT vertexBufferByteSize = 0;
	DXGI_FORMAT indexFormat = DXGI_FORMAT_R16_UINT;
	UINT indexBufferByteSize = 0;

	std::unordered_map<std::string, SubMeshGeometry> DrawArgs;

	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() const
	{
		D3D12_VERTEX_BUFFER_VIEW vbv;
		vbv.BufferLocation = vertexBufferGPU->GetGPUVirtualAddress();
		vbv.StrideInBytes = vertexByteStride;
		vbv.SizeInBytes = vertexBufferByteSize;

		return vbv;
	}

	D3D12_INDEX_BUFFER_VIEW GetIndexBufferView() const
	{
		D3D12_INDEX_BUFFER_VIEW ibv;
		ibv.BufferLocation = indexBufferGPU->GetGPUVirtualAddress();
		ibv.Format = indexFormat;
		ibv.SizeInBytes = indexBufferByteSize;

		return ibv;
	}

	void DisPosUploaders()
	{
		vertexUploadBuffer = nullptr;
		indexUploadBuffer = nullptr;
	}
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
	void SetGeometry(const MeshGeometry* geometry, string submeshName);
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

	const MeshGeometry* m_geo = nullptr;
	D3D12_PRIMITIVE_TOPOLOGY m_primitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	UINT m_indexCount = 0;
	UINT m_startIndexLocation = 0;
	UINT m_baseVertexLocation = 0;
};