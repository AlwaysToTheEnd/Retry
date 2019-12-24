#include "DX12FontMG.h"
#include "d3dUtil.h"
#include <SimpleMath.h>
#include <ResourceUploadBatch.h>


using namespace DirectX;

void DX12FontManager::Init(ID3D12Device* device, ID3D12CommandQueue* queue,
	const std::vector<std::wstring>& filePaths,
	DXGI_FORMAT rtFormat, DXGI_FORMAT dsForma)
{
	m_Memory = std::make_unique<GraphicsMemory>(device);

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = filePaths.size();
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	ThrowIfFailed(device->CreateDescriptorHeap(
		&heapDesc, IID_PPV_ARGS(m_DescriptorHeap.GetAddressOf())));

	ResourceUploadBatch resourceUpload(device);
	resourceUpload.Begin();

	RenderTargetState rtState(rtFormat, dsForma);
	SpriteBatchPipelineStateDescription pd(rtState);
	m_SpriteBatch = std::make_unique<SpriteBatch>(device, resourceUpload, pd);

	UINT descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	auto HeapCPUHandle = m_DescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	auto HeapGPUHandle = m_DescriptorHeap->GetGPUDescriptorHandleForHeapStart();

	std::wstring extension;
	for (auto& it : filePaths)
	{
		m_FontNames.push_back(GetFileNameFromPath(it, extension));
		m_Fonts.emplace_back(device, resourceUpload,
			it.c_str(), HeapCPUHandle, HeapGPUHandle);

		HeapCPUHandle.ptr += descriptorSize;
		HeapGPUHandle.ptr += descriptorSize;
	}

	auto uploadResourcesFinished = resourceUpload.End(queue);

	RenderFont::fontNames = m_FontNames;

	uploadResourcesFinished.wait();
}

void DX12FontManager::Resize(unsigned int clientWidth, unsigned clientHeight)
{
	if (m_SpriteBatch)
	{
		D3D12_VIEWPORT viewPort = { 0.0f,0.0f,
			static_cast<float>(clientWidth),static_cast<float>(clientHeight),
			D3D12_MIN_DEPTH,D3D12_MAX_DEPTH };

		m_SpriteBatch->SetViewport(viewPort);
	}
}

void DX12FontManager::RenderCommandWrite(ID3D12GraphicsCommandList* cmdList,
	const std::vector<RenderFont>& renderFonts)
{
	ID3D12DescriptorHeap* heaps[] = { m_DescriptorHeap.Get() };
	cmdList->SetDescriptorHeaps(_countof(heaps), heaps);

	m_SpriteBatch->Begin(cmdList);

	DirectX::SimpleMath::Vector2 origin = { 0,0 };
	DirectX::SimpleMath::Vector3 pos;
	DirectX::SimpleMath::Vector3 scale;
	DirectX::SimpleMath::Vector4 color;
	DirectX::SimpleMath::Vector2 size;

	for (auto& it : renderFonts)
	{
		pos = it.pos;
		color = it.color;
		scale = { 1,1,1 };

		size = m_Fonts[it.fontIndex].MeasureString(it.printString.c_str());

		if (it.fontHeight >= 0)
		{
			float scalePer = it.fontHeight/ size.y;
			scale.x = scalePer;
			scale.y = scalePer;

			pos.x -= (size.x * scale.x) / 2;
			pos.y -= (size.y * scale.y) / 2;

		}
		else
		{
			pos.x -= size.x / 2;
			pos.y -= size.y / 2;
		}

		m_Fonts[it.fontIndex].DrawString(m_SpriteBatch.get(), it.printString.c_str(), pos,
			color, it.rotation, origin, scale);
	}

	m_SpriteBatch->End();
}


