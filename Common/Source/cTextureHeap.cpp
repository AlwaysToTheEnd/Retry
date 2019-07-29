#include "cTextureHeap.h"

using namespace DirectX;
using namespace std;

cTextureHeap::cTextureHeap(ID3D12Device * device, UINT maxTexture)
	: m_SrvDescriptorSize(0)
{
	assert(device && maxTexture);

	m_device = device;
	m_SrvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	srvHeapDesc.NumDescriptors = maxTexture;

	ThrowIfFailed(device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(m_SrvHeap.GetAddressOf())));
}

void cTextureHeap::AddTexture(ID3D12CommandQueue* cmdqueue, const string& name, const wstring& filename)
{
	auto it = m_Textures.find(name);
	assert(it == m_Textures.end() && "This name is overlapping name");

	TEXTURENUM addTexture;
	addTexture.num = (UINT)m_Textures.size();
	addTexture.tex.name = name;

	ResourceUploadBatch resourceUpload(m_device);
	resourceUpload.Begin();

	size_t index = filename.find('.') + 1;
	wstring extension;
	extension.assign(&filename[index], filename.size() - index);

	if (extension == L"dds")
	{
		ThrowIfFailed(CreateDDSTextureFromFile(m_device,
			resourceUpload, filename.c_str(), addTexture.tex.resource.GetAddressOf()));
		m_Textures[name] = addTexture;
	}
	else
	{
		ThrowIfFailed(CreateWICTextureFromFile(m_device,
			resourceUpload, filename.c_str(), addTexture.tex.resource.GetAddressOf()));
		m_Textures[name] = addTexture;
	}

	auto uploadResourceFinished = resourceUpload.End(cmdqueue);
	uploadResourceFinished.wait();

	auto srvHeapHandle = (CD3DX12_CPU_DESCRIPTOR_HANDLE)m_SrvHeap->GetCPUDescriptorHandleForHeapStart();
	srvHeapHandle.Offset(addTexture.num, m_SrvDescriptorSize);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = addTexture.tex.resource->GetDesc().Format;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	srvDesc.Texture2D.MipLevels = addTexture.tex.resource->GetDesc().MipLevels;

	m_device->CreateShaderResourceView(addTexture.tex.resource.Get(), &srvDesc, srvHeapHandle);
}

void cTextureHeap::AddCubeMapTexture(ID3D12CommandQueue * cmdqueue, const string & name, const wstring & filename)
{
	auto it = m_Textures.find(name);
	assert(it == m_Textures.end() && "This name is overlapping name");

	TEXTURENUM addTexture;
	addTexture.num = (UINT)m_Textures.size();
	addTexture.tex.name = name;

	/*ResourceUploadBatch resourceUpload(m_device);
	resourceUpload.Begin();

	size_t index = filename.find('.') + 1;
	wstring extension;
	extension.assign(&filename[index], filename.size() - index);

	if (extension == L"dds")
	{
		ThrowIfFailed(CreateDDSTextureFromFile(m_device,
			resourceUpload, filename.c_str(), addTexture.tex.resource.GetAddressOf()));
		m_Textures[name] = addTexture;
	}
	else
	{
		assert(false && "Cube Texture support only dds extension");
	}

	auto uploadResourceFinished = resourceUpload.End(cmdqueue);
	uploadResourceFinished.wait();*/

	auto srvHeapHandle = (CD3DX12_CPU_DESCRIPTOR_HANDLE)m_SrvHeap->GetCPUDescriptorHandleForHeapStart();
	srvHeapHandle.Offset(addTexture.num, m_SrvDescriptorSize);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.Format = addTexture.tex.resource->GetDesc().Format;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	srvDesc.Texture2D.MipLevels = addTexture.tex.resource->GetDesc().MipLevels;

	m_device->CreateShaderResourceView(addTexture.tex.resource.Get(), &srvDesc, srvHeapHandle);
}

void cTextureHeap::AddNullTexture(const string & name, DXGI_FORMAT srvFormat,
	const D3D12_RESOURCE_DESC* resourceDesc, const D3D12_CLEAR_VALUE* optClear)
{
	auto it = m_Textures.find(name);
	assert(it == m_Textures.end() && "This name is overlapping name");

	TEXTURENUM addTexture;
	addTexture.num = (UINT)m_Textures.size();
	addTexture.tex.name = name;

	m_device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE, resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, optClear,
		IID_PPV_ARGS(addTexture.tex.resource.GetAddressOf()));

	m_Textures[name] = addTexture;

	auto srvHeapHandle = (CD3DX12_CPU_DESCRIPTOR_HANDLE)m_SrvHeap->GetCPUDescriptorHandleForHeapStart();
	srvHeapHandle.Offset(addTexture.num, m_SrvDescriptorSize);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = srvFormat;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	srvDesc.Texture2D.PlaneSlice = 0;

	m_device->CreateShaderResourceView(m_Textures[name].tex.resource.Get(), &srvDesc, srvHeapHandle);
}

ComPtr<ID3D12Resource> cTextureHeap::GetTexture(const string& name)
{
	auto it = m_Textures.find(name);
	assert(it != m_Textures.end() && "can not find this name");

	return it->second.tex.resource;
}

UINT cTextureHeap::GetTextureIndex(const string& name) const
{
	auto it = m_Textures.find(name);
	assert(it != m_Textures.end() && "can not find this name");

	return it->second.num;
}
