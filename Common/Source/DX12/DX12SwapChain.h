#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl.h>
#include <vector>
#include <DirectXColors.h>

#pragma comment(lib, "dxgi.lib")

class DX12SwapChain
{
public:
	DX12SwapChain();
	virtual ~DX12SwapChain();

	void CreateDXGIFactory(ID3D12Device** device);
	void CreateSwapChain(	HWND handle, ID3D12CommandQueue* queue, 
							DXGI_FORMAT renderTarget, DXGI_FORMAT depthStencil,
							unsigned int x, unsigned int y, unsigned int numSwapChain);
	
	void ReSize(ID3D12GraphicsCommandList* cmd, unsigned int x, unsigned int y);
	void RenderBegin(ID3D12GraphicsCommandList* cmd, const float clearColor[4]);
	void RenderEnd(ID3D12GraphicsCommandList* cmd);
	void Presnet();

	void GetRenderTargetFormats(std::vector<DXGI_FORMAT>& out);

private:
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const;

private:
	ID3D12Device*											m_Device;
	DXGI_FORMAT												m_BackBufferFormat;
	DXGI_FORMAT												m_DepthStencilFormat;
	size_t													m_NumSwapBuffer;
	size_t													m_CurrBackBuffer;
	unsigned int											m_RTVDescriptorSize;
	unsigned int											m_DSVDescriptorSize;

	Microsoft::WRL::ComPtr<IDXGIFactory4>					m_DxgiFactory;
	Microsoft::WRL::ComPtr<IDXGISwapChain>					m_SwapChain;

	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>>		m_RenderTargetBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource>					m_DepthStencilBuffer;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>			m_RTVHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>			m_DSVHeap;
	float													zero[4] = {};
};