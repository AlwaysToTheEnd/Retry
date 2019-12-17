#pragma once
#include <vector>
#include <unordered_map>
#include <d3d12.h>
#include <memory>
#include <wrl.h>
#include "DX12RenderClasses.h"
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <GraphicsMemory.h>


using Microsoft::WRL::ComPtr;


class DX12FontManager
{
public:
	DX12FontManager() = default;
	virtual ~DX12FontManager() =default;

	void Init(ID3D12Device* device, ID3D12CommandQueue* queue, 
				const std::vector<std::wstring>& filePaths,
				DXGI_FORMAT rtFormat, DXGI_FORMAT dsFormat);

	void Resize(unsigned int clientWidth, unsigned clientHeight);
	void RenderCommandWrite(ID3D12GraphicsCommandList* cmdList, 
		const std::vector<RenderFont>& renderFonts);

	void Commit(ID3D12CommandQueue* queue) { m_Memory->Commit(queue); }

private:
	ComPtr<ID3D12DescriptorHeap>				m_DescriptorHeap;
	std::unique_ptr<DirectX::SpriteBatch>		m_SpriteBatch;
	std::unique_ptr<DirectX::GraphicsMemory>	m_Memory;

	std::vector<DirectX::SpriteFont>			m_Fonts;
	std::vector<std::wstring>					m_FontNames;
};

