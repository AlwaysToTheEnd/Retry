#include "cTextureBuffer.h"
#include "d3dx12.h"
#include "DirectXTex.h"
#include "d3dUtil.h"

using namespace DirectX;
using namespace std;

cTextureBuffer::cTextureBuffer(ID3D12Device* device, UINT maxTexture)
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

void cTextureBuffer::Begin(ID3D12Device* device)
{
	assert(!m_isBeging && "TextureHeap aleady beging state");

	ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(m_comAlloc.GetAddressOf())));

	ThrowIfFailed(device->CreateCommandList(1, D3D12_COMMAND_LIST_TYPE_DIRECT, m_comAlloc.Get(),
		nullptr, IID_PPV_ARGS(m_commandList.GetAddressOf())));

	m_isBeging = true;
}

void cTextureBuffer::AddTexture(ID3D12Device* device, ID3D12CommandQueue* cmdqueue, const wstring& filename)
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

void cTextureBuffer::AddCubeMapTexture(ID3D12Device* device, 
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

void cTextureBuffer::AddNullTexture(ID3D12Device* device, const string& name, DXGI_FORMAT srvFormat,
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

void cTextureBuffer::End(ID3D12CommandQueue* queue, function<void()> flushCommandQueueFunc)
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

ComPtr<ID3D12Resource> cTextureBuffer::GetTexture(const string& name)
{
	auto it = m_Textures.find(name);
	assert(it != m_Textures.end() && "can not find this name");

	return it->second.tex.resource;
}

UINT cTextureBuffer::GetTextureIndex(const string& name) const
{
	auto it = m_Textures.find(name);
	assert(it != m_Textures.end() && "can not find this name");

	return it->second.num;
}

void cTextureBuffer::DataToDevice(
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
	uint8_t* mapedPtr = nullptr;

	textureDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
	textureDesc.DepthOrArraySize= metadata.arraySize;
	textureDesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metadata.dimension);
	textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	textureDesc.Format = metadata.format;
	textureDesc.Height = metadata.height;
	textureDesc.Width = metadata.width;
	textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	textureDesc.MipLevels = metadata.mipLevels;
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

	ThrowIfFailed(target.tex.uploadHeap->Map(0, &range, reinterpret_cast<void**>(&mapedPtr)));
	std::memcpy(mapedPtr, pixelMemory, range.End);
	target.tex.uploadHeap->Unmap(0, &range);

	D3D12_TEXTURE_COPY_LOCATION dest;
	dest.pResource = target.tex.resource.Get();
	dest.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	dest.SubresourceIndex = 0;

	uploaderDesc = target.tex.uploadHeap->GetDesc();
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footPrint;
	device->GetCopyableFootprints(&uploaderDesc, 0, 1, 0, &footPrint, nullptr, nullptr, nullptr);

	D3D12_TEXTURE_COPY_LOCATION src;
	src.pResource = target.tex.uploadHeap.Get();
	src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	src.PlacedFootprint = footPrint;
	src.PlacedFootprint.Footprint.Width = footPrint.Footprint.RowPitch;
	src.PlacedFootprint.Footprint.Format = textureDesc.Format;
	m_commandList->CopyTextureRegion(&dest, 0, 0, 0, &src, nullptr);
}