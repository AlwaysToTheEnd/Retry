#include "DX12FontMG.h"
#include "d3dUtil.h"
#include <SimpleMath.h>
#include <ResourceUploadBatch.h>
#include <d3dx12.h>
#include <d3dcompiler.h>


using namespace DirectX;

void DX12FontManager::Init(ID3D12Device* device, ID3D12CommandQueue* queue,
	const std::vector<std::wstring>& filePaths,
	DXGI_FORMAT rtFormat, DXGI_FORMAT dsForma)
{
	m_Memory = std::make_unique<GraphicsMemory>(device);
	m_States = std::make_unique<DirectX::CommonStates>(device);
	m_ObjectIDUploadBuffer = std::make_unique<DX12UploadBuffer<int>>(device, 500, true);

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = CGH::SizeTTransUINT(filePaths.size());
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	ThrowIfFailed(device->CreateDescriptorHeap(
		&heapDesc, IID_PPV_ARGS(m_DescriptorHeap.GetAddressOf())));

	ResourceUploadBatch resourceUpload(device);
	resourceUpload.Begin();

	D3D12_RENDER_TARGET_BLEND_DESC transparencyBlendDesc;
	transparencyBlendDesc.BlendEnable = true;
	transparencyBlendDesc.LogicOpEnable = false;
	transparencyBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	transparencyBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	transparencyBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	transparencyBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	transparencyBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	transparencyBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	transparencyBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
	transparencyBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	
	RenderTargetState rtState(rtFormat, dsForma);
	rtState.numRenderTargets = 2;
	rtState.rtvFormats[1] = DXGI_FORMAT_R32_SINT;
	SpriteBatchPipelineStateDescription pd(rtState);
	pd.depthStencilDesc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	pd.blendDesc = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	pd.blendDesc.AlphaToCoverageEnable = false;
	pd.blendDesc.IndependentBlendEnable = true;
	pd.blendDesc.RenderTarget[0] = transparencyBlendDesc;
	pd.samplerDescriptor = m_States->AnisotropicWrap();

	{
		UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
		compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
		HRESULT hr = S_OK;
		ComPtr<ID3DBlob> errors;
		hr = D3DCompileFromFile(L"../Common/MainShaders/FontPixelShader.hlsl", nullptr,
			D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_1", compileFlags, 0, m_PixelShader.GetAddressOf(), &errors);

		if (errors != nullptr)
		{
			OutputDebugStringA((char*)errors->GetBufferPointer());
		}

		ThrowIfFailed(hr);

		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;

		CD3DX12_DESCRIPTOR_RANGE textureSRV(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		CD3DX12_DESCRIPTOR_RANGE textureSampler(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0);
		CD3DX12_ROOT_PARAMETER customRootParam[ROOT_COUNT];
		customRootParam[ROOT_TEXTURE_SRV].InitAsDescriptorTable(1, &textureSRV, D3D12_SHADER_VISIBILITY_PIXEL);
		customRootParam[ROOT_CBBUFFER_CB].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);
		customRootParam[ROOT_TEXTURE_SAMPLER].InitAsDescriptorTable(1, &textureSampler, D3D12_SHADER_VISIBILITY_PIXEL);
		customRootParam[ROOT_OBJECTID_CB].InitAsConstantBufferView(1, 0);

		CD3DX12_ROOT_SIGNATURE_DESC customRoot;
		customRoot.Init(ROOT_COUNT, customRootParam, 0, nullptr, rootSignatureFlags);

		ComPtr<ID3DBlob> serializedRootSig = nullptr;
		ComPtr<ID3DBlob> error = nullptr;

		hr = D3D12SerializeRootSignature(&customRoot, D3D_ROOT_SIGNATURE_VERSION_1,
			serializedRootSig.GetAddressOf(), error.GetAddressOf());

		if (error != nullptr)
		{
			::OutputDebugStringA((char*)error->GetBufferPointer());
		}
		ThrowIfFailed(hr);

		ThrowIfFailed(device->CreateRootSignature(0, serializedRootSig->GetBufferPointer(),
			serializedRootSig->GetBufferSize(), IID_PPV_ARGS(m_RootSignature.GetAddressOf())));
	}
	pd.customPixelShader.BytecodeLength = m_PixelShader->GetBufferSize();
	pd.customPixelShader.pShaderBytecode = m_PixelShader->GetBufferPointer();
	pd.customRootSignature = m_RootSignature.Get();

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
	ID3D12DescriptorHeap* heaps[] = { m_DescriptorHeap.Get(), m_States->Heap() };
	cmdList->SetDescriptorHeaps(_countof(heaps), heaps);

	m_SpriteBatch->Begin(cmdList, SpriteSortMode_Immediate);
	
	DirectX::SimpleMath::Vector2 origin = { 0,0 };
	DirectX::SimpleMath::Vector3 pos;
	DirectX::SimpleMath::Vector3 scale;
	DirectX::SimpleMath::Vector4 color;
	DirectX::SimpleMath::Vector2 size;

	unsigned int index = 0;
	unsigned int elementByteSize = m_ObjectIDUploadBuffer->GetElementByteSize();
	auto uploadbufferAdress = m_ObjectIDUploadBuffer->Resource()->GetGPUVirtualAddress();
	
	for (auto& it : renderFonts)
	{
		m_ObjectIDUploadBuffer->CopyData(index, it.pixelColID);
		index++;
	}

	for (auto& it : renderFonts)
	{
		cmdList->SetGraphicsRootConstantBufferView(ROOT_OBJECTID_CB, uploadbufferAdress);

		pos = { it.pos.x,it.pos.y,it.pos.z };
		color = { it.color.x, it.color.y, it.color.z, it.color.w };
		scale = { 1,1,1 };

		size = m_Fonts[it.fontIndex].MeasureString(it.printString.c_str());

		if (it.fontHeight >= 0)
		{
			float scalePer = it.fontHeight / size.y;
			scale.x = scalePer;
			scale.y = scalePer;

			size.x *= scale.x;
			size.y *= scale.y;
		}

		if (it.drawSize)
		{
			*it.drawSize = { size.x, size.y };
		}


		pos.x -= size.x * it.benchmark.x;
		pos.y -= size.y * it.benchmark.y;
		
		m_Fonts[it.fontIndex].DrawString(m_SpriteBatch.get(), it.printString.c_str(), pos,
			color, it.rotation, origin, scale, DirectX::SpriteEffects_None, pos.z);

		uploadbufferAdress += elementByteSize;
	}

	m_SpriteBatch->End();
}


