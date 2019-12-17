#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <d3d12.h>
#include <DirectXMath.h>
#include <memory>
#include <wrl.h>

using Microsoft::WRL::ComPtr;

namespace DirectX
{
	class SpriteBatch;
	class SpriteFont;
}

struct RenderFont
{
	unsigned int fontIndex = 0;
 	const std::wstring& printString;
	float rotation = 0;
	DirectX::XMFLOAT3 pos = { 0, 0, 0 };
	DirectX::XMFLOAT3 scale = { 1, 1, 1 };
	DirectX::XMFLOAT4 color = { 0,0,0,0 };
};

class DX12FontManager
{
public:
	DX12FontManager() = default;
	virtual ~DX12FontManager() =default;

	void Init(ID3D12Device* device, ID3D12CommandQueue* queue, 
				const std::vector<std::wstring>& filePaths,
				DXGI_FORMAT rtFormat, DXGI_FORMAT dsFormat);

	void Resize(unsigned int clientWidth, unsigned clientHeight);
	void RenderCommandWrite(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderFont>& renderFonts);

private:
	ComPtr<ID3D12DescriptorHeap>			m_DescriptorHeap = nullptr;
	std::unique_ptr<DirectX::SpriteBatch>	m_SpriteBatch = nullptr;

	std::vector<DirectX::SpriteFont>		m_Fonts;
	std::vector<std::wstring>				m_FontNames;
};

