#pragma once
#include <vector>
#include <memory>
#include "d3dUtil.h"
#include "Vertex.h"
#include "BaseClass.h"
#include "DX12UploadBuffer.h"

struct DX12HeightFieldMesh
{
	DX12HeightFieldMesh(ID3D12Device* device, unsigned int numHeights)
		: scale(1,1,1)
		, offsetPos(0,0,0)
	{
		heights = std::make_unique<DX12UploadBuffer<float>>(device, numHeights, false);
		
		D3D12_HEAP_PROPERTIES vhp = {};
		vhp.Type = D3D12_HEAP_TYPE_DEFAULT;
		vhp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE;
		vhp.MemoryPoolPreference = D3D12_MEMORY_POOL_L1;
		vhp.CreationNodeMask = 1;
		vhp.VisibleNodeMask = 1;

		D3D12_RESOURCE_DESC vhrd = {};
		vhrd.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		vhrd.Width = numHeights * sizeof(Vertex);
		vhrd.Height = 1;
		vhrd.DepthOrArraySize = 1;
		vhrd.MipLevels = 1;
		vhrd.Format = DXGI_FORMAT_UNKNOWN;
		vhrd.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		vhrd.SampleDesc.Count = 1;
		vhrd.SampleDesc.Quality = 0;

		ThrowIfFailed(device->CreateCommittedResource(&vhp, D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS, &vhrd, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			nullptr, IID_PPV_ARGS(vertices.GetAddressOf())));
	}

	std::unique_ptr<DX12UploadBuffer<float>>	heights;
	Microsoft::WRL::ComPtr<ID3D12Resource>		vertices;
	physx::PxVec3								scale;
	physx::PxVec3								offsetPos;
};