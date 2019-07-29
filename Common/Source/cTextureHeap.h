#pragma once
#include <wrl.h>
#include <d3d12.h>
#include "d3dUtil.h"

using Microsoft::WRL::ComPtr;

class cTextureHeap
{
public:
	cTextureHeap() = delete;
	cTextureHeap(cTextureHeap& rhs) = delete;
	cTextureHeap& operator=(const cTextureHeap& rhs) = delete;
	cTextureHeap(ID3D12Device* device ,UINT maxTexture);
	
	void AddTexture(ID3D12CommandQueue* cmdqueue, const std::string& name, const std::wstring& filename);
	void AddCubeMapTexture(ID3D12CommandQueue* cmdqueue, const std::string& name, const std::wstring& filename);
	void AddNullTexture(const std::string& name, DXGI_FORMAT srvFormat,
		const D3D12_RESOURCE_DESC* resourceDesc,const D3D12_CLEAR_VALUE* optClear);

	ID3D12DescriptorHeap* GetHeap() { return m_SrvHeap.Get(); }
	ComPtr<ID3D12Resource> GetTexture(const std::string& name);
	
	UINT GetTextureIndex(const std::string& name) const;
	UINT GetTexturesNum() const { return (UINT)m_Textures.size(); }

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

	ID3D12Device* m_device;
	UINT m_SrvDescriptorSize;
	ComPtr<ID3D12DescriptorHeap> m_SrvHeap;
	std::unordered_map<std::string, TEXTURENUM> m_Textures;
};