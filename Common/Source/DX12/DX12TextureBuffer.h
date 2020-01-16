#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <functional>
#include <unordered_map>
#include <string>

using Microsoft::WRL::ComPtr;

//important reference
//https://github.com/Microsoft/DirectXTex/wiki/DirectXTex

namespace DirectX
{
	struct TexMetadata;
	class ScratchImage;
}

class DX12TextureBuffer
{
private:
	struct Texture
	{
		ComPtr<ID3D12Resource> resource = nullptr;
		ComPtr<ID3D12Resource> uploadHeap = nullptr;
	};

	struct TEXTURENUM
	{
		UINT num = 0;
		DX12TextureBuffer::Texture tex;
	};

	enum TEXTURE_FILE_TYPE
	{
		DDS_TEXTURE,
		WIC_TEXTURE,
		NONE_TEXTURE,
	};

public:
	DX12TextureBuffer() = delete;
	DX12TextureBuffer(DX12TextureBuffer& rhs) = delete;
	DX12TextureBuffer& operator=(const DX12TextureBuffer& rhs) = delete;
	DX12TextureBuffer(ID3D12Device* device, UINT maxTexture);
	
	void Begin(ID3D12Device* device);
	void AddTexture(ID3D12Device* device, ID3D12CommandQueue* cmdqueue, const std::wstring& filename);
	void AddCubeMapTexture(ID3D12Device* device, ID3D12CommandQueue* cmdqueue, const std::string& name, const std::wstring& filename);
	void AddNullTexture(ID3D12Device* device, const std::string& name, DXGI_FORMAT srvFormat,
		const D3D12_RESOURCE_DESC* resourceDesc,const D3D12_CLEAR_VALUE* optClear);
	void End(ID3D12CommandQueue* queue, std::function<void()> flushCommandQueueFunc);

	ID3D12DescriptorHeap* GetHeap() { return m_SrvHeap.Get(); }
	ComPtr<ID3D12Resource> GetTexture(const std::string& name);
	
	int GetTextureIndex(const std::string& name) const;
	UINT GetTexturesNum() const { return (UINT)m_Textures.size(); }

private:
	void DataToDevice(ID3D12Device* device, const DirectX::TexMetadata& metadata, const DirectX::ScratchImage& scratch, TEXTURENUM& target);

private:
	ComPtr<ID3D12GraphicsCommandList>			m_commandList;
	ComPtr<ID3D12CommandAllocator>				m_comAlloc;
	UINT										m_SrvDescriptorSize;
	bool										m_isBeging;

	ComPtr<ID3D12DescriptorHeap>				m_SrvHeap;
	std::unordered_map<std::string, TEXTURENUM> m_Textures;
};