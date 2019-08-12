#pragma once
#include <wrl.h>
#include <d3d12.h>
#include "d3dUtil.h"

using Microsoft::WRL::ComPtr;
struct IWICBitmapFrameDecode;
struct IWICImagingFactory2;

enum WIC_LOADER_FLAGS : uint32_t
{
	WIC_LOADER_DEFAULT = 0,
	WIC_LOADER_FORCE_SRGB = 0x1,
	WIC_LOADER_IGNORE_SRGB = 0x2,
	WIC_LOADER_MIP_AUTOGEN = 0x4,
	WIC_LOADER_MIP_RESERVE = 0x8,
};

class cTextureHeap
{
public:
	cTextureHeap() = delete;
	cTextureHeap(cTextureHeap& rhs) = delete;
	cTextureHeap& operator=(const cTextureHeap& rhs) = delete;
	cTextureHeap(ID3D12Device* device, UINT maxTexture);
	
	void Begin(ID3D12Device* device);
	void AddTexture(ID3D12Device* device, ID3D12CommandQueue* cmdqueue, const std::string& name, const std::wstring& filename);
	void AddCubeMapTexture(ID3D12Device* device, ID3D12CommandQueue* cmdqueue, const std::string& name, const std::wstring& filename);
	void AddNullTexture(ID3D12Device* device, const std::string& name, DXGI_FORMAT srvFormat,
		const D3D12_RESOURCE_DESC* resourceDesc,const D3D12_CLEAR_VALUE* optClear);
	void End(ID3D12CommandQueue* queue, void(*flushCommandQueueFunc)());

	ID3D12DescriptorHeap* GetHeap() { return m_SrvHeap.Get(); }
	ComPtr<ID3D12Resource> GetTexture(const std::string& name);
	
	UINT GetTextureIndex(const std::string& name) const;
	UINT GetTexturesNum() const { return (UINT)m_Textures.size(); }

private:
	HRESULT LoadWICTexture(ID3D12Device* device, const std::wstring& filename,size_t maxsize,
		D3D12_RESOURCE_FLAGS resFlags, WIC_LOADER_FLAGS loadflags, ID3D12Resource** texture);
	bool IsDDSTextureFile(const std::wstring& filename);

private:
	struct Texture
	{
		std::string name;
		ComPtr<ID3D12Resource> resource = nullptr;
		ComPtr<ID3D12Resource> uploadHeap = nullptr;
	};

	struct TEXTURENUM
	{
		UINT num=0;
		Texture tex;
	};

	enum TEXTURE_FILE_TYPE
	{
		DDS_TEXTURE,
		WIC_TEXTURE,
		NONE_TEXTURE,
	};
	
	ComPtr<ID3D12GraphicsCommandList>			m_commandList;
	ComPtr<ID3D12CommandAllocator>				m_comAlloc;
	UINT										m_SrvDescriptorSize;
	bool										m_isBeging;

	ComPtr<ID3D12DescriptorHeap>				m_SrvHeap;
	std::unordered_map<std::string, TEXTURENUM> m_Textures;
};