#include <DirectXMath.h>
#include "DX12ShadowMap.h"
#include "DX12RenderClasses.h"
#include "d3dUtil.h"

using namespace DirectX;

Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DX12ShadowMap::m_DsvHeap = nullptr;
unsigned int DX12ShadowMap::m_DsvSize = 0;
unsigned int DX12ShadowMap::m_CurrShadowMapNum = 0;

void DX12ShadowMap::Resize(ID3D12Device* device, unsigned int width, unsigned int height)
{
	if (m_Resource)
	{
		if (m_Height != height || m_Width != width)
		{
			CreateResource(device, width, height);
		}
	}
	else
	{
		CreateResource(device, width, height);
	}
}

void DX12ShadowMap::ShadowMapDsvHeapCreate(ID3D12Device* device)
{
	m_DsvSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	D3D12_DESCRIPTOR_HEAP_DESC desc;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NodeMask = 1;
	desc.NumDescriptors = MAXLIGHT;

	ThrowIfFailed(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(m_DsvHeap.GetAddressOf())));
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12ShadowMap::GetDsv() const
{
	auto dsvHeapPtr = m_DsvHeap->GetCPUDescriptorHandleForHeapStart();
	dsvHeapPtr.ptr += m_DsvSize * m_Index;
	return dsvHeapPtr;
}

void DX12ShadowMap::GetShadowMatrix(const physx::PxVec3& startSpot, const physx::PxVec3& dir,
	const physx::PxMat44& projection, physx::PxMat44& out)
{
	XMVECTOR look = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&startSpot));
	XMVECTOR at = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&dir));

	XMMATRIX lookAtMat = XMMatrixLookAtLH(look, at, XMVectorSet(0, 1, 0, 0));
	XMMATRIX proj = XMLoadFloat4x4(projection);

	XMMATRIX NDCtoTEX(	0.5f,		0,		0,		0,
						0,		-0.5f,		0,		0,
						0,			0,	 1.0f,		0,
						0.5f,	 0.5f,		0,	 1.0f);

	XMMATRIX shadowMat = lookAtMat * proj * NDCtoTEX;
	XMStoreFloat4x4(out, shadowMat);
}

void DX12ShadowMap::CreateResource(ID3D12Device* device, unsigned int width, unsigned int height)
{
	m_Resource = nullptr;

	m_Height = height;
	m_Width = width;

	D3D12_HEAP_PROPERTIES properties = {};
	properties.Type = D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT;
	properties.VisibleNodeMask = 1;
	properties.CreationNodeMask = 1;

	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	desc.Width = width;
	desc.Height = height;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	ThrowIfFailed(device->CreateCommittedResource(&properties, D3D12_HEAP_FLAG_NONE, &desc,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(m_Resource.GetAddressOf())));

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = desc.Format;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 1;

	auto dsvHeapPtr = m_DsvHeap->GetCPUDescriptorHandleForHeapStart();
	dsvHeapPtr.ptr += m_DsvSize * m_Index;
	device->CreateDepthStencilView(m_Resource.Get(), &dsvDesc, dsvHeapPtr);
}
