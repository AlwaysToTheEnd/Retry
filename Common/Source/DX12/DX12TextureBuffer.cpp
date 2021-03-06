#include "DX12TextureBuffer.h"
#include "d3dx12.h"
#include "DirectXTex.h"
#include "d3dUtil.h"

using namespace DirectX;
using namespace std;

DX12TextureBuffer::DX12TextureBuffer(ID3D12Device* device, UINT maxTexture)
	: m_SrvDescriptorSize(0)
	, m_isBeging(false)
{
	assert(device && maxTexture);

	m_SrvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	srvHeapDesc.NumDescriptors = maxTexture;

	ThrowIfFailed(device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(m_SrvHeap.GetAddressOf())));
}

void DX12TextureBuffer::Begin(ID3D12Device* device)
{
	assert(!m_isBeging && "TextureHeap aleady beging state");

	ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(m_comAlloc.GetAddressOf())));

	ThrowIfFailed(device->CreateCommandList(1, D3D12_COMMAND_LIST_TYPE_DIRECT, m_comAlloc.Get(),
		nullptr, IID_PPV_ARGS(m_commandList.GetAddressOf())));

	m_isBeging = true;
}

void DX12TextureBuffer::AddTexture(ID3D12Device* device, ID3D12CommandQueue* cmdqueue, const wstring& filename)
{
	assert(m_isBeging && "can not call to create texture on a closed TextureBuffer");

	wstring extension;
	wstring wname = GetFileNameFromPath(filename, extension);
	string name(wname.begin(), wname.end());

	auto it = m_Textures.find(name);
	assert(it == m_Textures.end() && "This name is overlapping name");

	TEXTURENUM addedTexture;
	addedTexture.num = (UINT)m_Textures.size();

	TexMetadata metaData;
	ScratchImage scratch;

	if (extension == L"dds")
	{
		ThrowIfFailed(LoadFromDDSFile(filename.c_str(), DDS_FLAGS_NONE ,&metaData, scratch));
	}
	else if (extension == L"tga")
	{
		ThrowIfFailed(LoadFromTGAFile(filename.c_str(), &metaData, scratch));
	}
	else
	{
		ThrowIfFailed(LoadFromWICFile(filename.c_str(), WIC_FLAGS_NONE ,&metaData, scratch));
	}

	DataToDevice(device, metaData, scratch, addedTexture);
	m_Textures.insert({ name, addedTexture });

	wstring wTextureName = L"Texture_";
	wTextureName.assign(name.begin(), name.end());
	addedTexture.tex.resource->SetName(wTextureName.c_str());

	auto srvHeapHandle = (CD3DX12_CPU_DESCRIPTOR_HANDLE)m_SrvHeap->GetCPUDescriptorHandleForHeapStart();
	srvHeapHandle.Offset(addedTexture.num, m_SrvDescriptorSize);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = addedTexture.tex.resource->GetDesc().Format;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	srvDesc.Texture2D.MipLevels = addedTexture.tex.resource->GetDesc().MipLevels;

	device->CreateShaderResourceView(addedTexture.tex.resource.Get(), &srvDesc, srvHeapHandle);
}

void DX12TextureBuffer::AddCubeMapTexture(ID3D12Device* device, 
	ID3D12CommandQueue* cmdqueue, const string& name, const wstring& filename)
{
	auto it = m_Textures.find(name);
	assert(it == m_Textures.end() && "This name is overlapping name");

	TEXTURENUM addedTexture;
	addedTexture.num = (UINT)m_Textures.size();

	wstring extension;
	GetFileNameFromPath(filename, extension);
	//if (extension == L"dds")
	//{
	//	ThrowIfFailed(CreateDDSTextureFromFile12(device, m_commandList.Get(), filename.c_str(),
	//		addedTexture.tex.resource, addedTexture.tex.uploadHeap));
	//	m_Textures[name] = addedTexture;
	//}
	//else
	//{
	//	assert(false && "Cube Texture is supported only dds extension");
	//}

	auto srvHeapHandle = (CD3DX12_CPU_DESCRIPTOR_HANDLE)m_SrvHeap->GetCPUDescriptorHandleForHeapStart();
	srvHeapHandle.Offset(addedTexture.num, m_SrvDescriptorSize);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.Format = addedTexture.tex.resource->GetDesc().Format;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	srvDesc.Texture2D.MipLevels = addedTexture.tex.resource->GetDesc().MipLevels;

	device->CreateShaderResourceView(addedTexture.tex.resource.Get(), &srvDesc, srvHeapHandle);
}

void DX12TextureBuffer::AddNullTexture(ID3D12Device* device, const string& name, DXGI_FORMAT srvFormat,
	const D3D12_RESOURCE_DESC* resourceDesc, const D3D12_CLEAR_VALUE* optClear)
{
	auto it = m_Textures.find(name);
	assert(it == m_Textures.end() && "This name is overlapping name");

	TEXTURENUM addTexture;
	addTexture.num = (UINT)m_Textures.size();

	device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE, resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, optClear,
		IID_PPV_ARGS(addTexture.tex.resource.GetAddressOf()));

	//m_Textures[name] = addTexture;

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

	device->CreateShaderResourceView(m_Textures[name].tex.resource.Get(), &srvDesc, srvHeapHandle);
}

void DX12TextureBuffer::End(ID3D12CommandQueue* queue, function<void()> flushCommandQueueFunc)
{
	assert(m_isBeging && "TextureHeap already closed");

	ThrowIfFailed(m_commandList->Close());

	ID3D12CommandList* commandLists[] = { m_commandList.Get() };
	queue->ExecuteCommandLists(1, commandLists);
	flushCommandQueueFunc();

	for (auto& it : m_Textures)
	{
		it.second.tex.uploadHeap = nullptr;
	}

	m_commandList.Reset();
	m_comAlloc.Reset();
}

ComPtr<ID3D12Resource> DX12TextureBuffer::GetTexture(const string& name)
{
	auto it = m_Textures.find(name);
	assert(it != m_Textures.end() && "can not find this name");

	return it->second.tex.resource;
}

int DX12TextureBuffer::GetTextureIndex(const string& name) const
{
	auto it = m_Textures.find(name);
	if (it == m_Textures.end())
	{
		return -1;
	}
	
	return it->second.num;
}

void DX12TextureBuffer::DataToDevice(
	ID3D12Device* device,
	const DirectX::TexMetadata& metadata, 
	const DirectX::ScratchImage& scratch, 
	TEXTURENUM& target)
{
	const uint8_t* pixelMemory = scratch.GetPixels();
	const Image* image = scratch.GetImages();
	const size_t imageNum = scratch.GetImageCount();

	CD3DX12_HEAP_PROPERTIES textureHP(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_HEAP_PROPERTIES uploadHP(D3D12_HEAP_TYPE_UPLOAD);
	D3D12_RESOURCE_DESC textureDesc = {};
	D3D12_RESOURCE_DESC uploaderDesc = {};
	CD3DX12_RANGE range(0, scratch.GetPixelsSize());

	textureDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
	textureDesc.DepthOrArraySize= static_cast<UINT16>(metadata.arraySize);
	textureDesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metadata.dimension);
	textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	textureDesc.Format = metadata.format;
	textureDesc.Height = static_cast<UINT>(metadata.height);
	textureDesc.Width = static_cast<UINT>(metadata.width);
	textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	textureDesc.MipLevels = static_cast<UINT16>(metadata.mipLevels);
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;

	ThrowIfFailed(device->CreateCommittedResource(
		&textureHP,
		D3D12_HEAP_FLAG_NONE,
		&textureDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(target.tex.resource.GetAddressOf())));

	uploaderDesc = textureDesc;

	uploaderDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	uploaderDesc.Height = 1;
	uploaderDesc.Width = range.End;
	uploaderDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	uploaderDesc.Format = DXGI_FORMAT_UNKNOWN;

	ThrowIfFailed(device->CreateCommittedResource(
		&uploadHP,
		D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
		&uploaderDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(target.tex.uploadHeap.GetAddressOf())));

	D3D12_SUBRESOURCE_DATA subData;
	subData.pData = pixelMemory;
	subData.RowPitch = image->rowPitch;
	subData.SlicePitch = image->slicePitch;
	UpdateSubresources<1>(m_commandList.Get(), target.tex.resource.Get(), 
		target.tex.uploadHeap.Get(), 0, 0, 1, &subData);

}