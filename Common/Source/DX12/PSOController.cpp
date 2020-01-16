#include "PSOController.h"
#include "d3dUtil.h"
#include <D3Dcompiler.h>
#pragma comment(lib,"d3dcompiler.lib")

using namespace std;

PSOController::PSOController(ID3D12Device* device)
	:m_Device(device)
{
}

PSOController::~PSOController()
{

}

void PSOController::SetPSOToCommnadList(ID3D12GraphicsCommandList* cmd,
	const std::vector<DXGI_FORMAT>& rtvFormats, DXGI_FORMAT dsvFormat, D3D12_PRIMITIVE_TOPOLOGY_TYPE primitive,
	const std::string& input, const std::string& rootSig,
	const std::string& rasterizer, const std::string& blend, const std::string& depthStencil,
	const std::string& vs, const std::string& ps,
	const std::string& gs, const std::string& hs, const std::string& ds)
{
	std::string keyName = input + ',' +
		rootSig + ',' +
		rasterizer + ',' +
		blend + ',' +
		depthStencil + ',' +
		vs + ',' + ps + ',' +
		gs + ',' + hs + ',' + ds + ',' +
		to_string(primitive) + ',' + to_string(dsvFormat) + ',';

	for (auto& it : rtvFormats)
	{
		keyName += to_string(it) + ',';
	}

	auto psoI = m_PSOs.find(keyName);
	auto rootSigI = m_RootSignature.find(rootSig);

	if (psoI == m_PSOs.end())
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC newPSO = {};

		auto inputI = m_InputLayouts.find(input);

		auto rasterI = m_Rasterizers.find(rasterizer);
		auto blendI = m_Blends.find(blend);
		auto depthStencilI = m_DepthStencils.find(depthStencil);

		auto vsI = m_Shaders[DX12_SHADER_VERTEX].find(vs);
		auto psI = m_Shaders[DX12_SHADER_PIXEL].find(ps);

		assert(rootSigI != m_RootSignature.end());
		assert(inputI != m_InputLayouts.end());
		assert(rasterI != m_Rasterizers.end());
		assert(blendI != m_Blends.end());
		assert(depthStencilI != m_DepthStencils.end());
		assert(vsI != m_Shaders[DX12_SHADER_VERTEX].end());
		assert(psI != m_Shaders[DX12_SHADER_PIXEL].end());

		D3D12_INPUT_LAYOUT_DESC inputDesc;
		inputDesc.NumElements = inputI->second.size();
		inputDesc.pInputElementDescs = inputI->second.data();
		newPSO.InputLayout = inputDesc;
		newPSO.pRootSignature = rootSigI->second.Get();

		newPSO.RasterizerState = rasterI->second;
		newPSO.BlendState = blendI->second;
		newPSO.DepthStencilState = depthStencilI->second;

		newPSO.PrimitiveTopologyType = primitive;
		newPSO.DSVFormat = dsvFormat;
		newPSO.NumRenderTargets = rtvFormats.size();
		memcpy(newPSO.RTVFormats, rtvFormats.data(), rtvFormats.size() * sizeof(DXGI_FORMAT));

		newPSO.VS.BytecodeLength = vsI->second.shader->GetBufferSize();
		newPSO.VS.pShaderBytecode = vsI->second.shader->GetBufferPointer();

		newPSO.PS.BytecodeLength = psI->second.shader->GetBufferSize();
		newPSO.PS.pShaderBytecode = psI->second.shader->GetBufferPointer();

		if (gs.size())
		{
			auto gsI = m_Shaders[DX12_SHADER_GEOMETRY].find(gs);
			assert(gsI != m_Shaders[DX12_SHADER_GEOMETRY].end());

			newPSO.GS.BytecodeLength = gsI->second.shader->GetBufferSize();
			newPSO.GS.pShaderBytecode = gsI->second.shader->GetBufferPointer();
		}

		if (hs.size())
		{
			auto hsI = m_Shaders[DX12_SHADER_HULL].find(hs);
			assert(hsI != m_Shaders[DX12_SHADER_HULL].end());

			newPSO.HS.BytecodeLength = hsI->second.shader->GetBufferSize();
			newPSO.HS.pShaderBytecode = hsI->second.shader->GetBufferPointer();
		}

		if (ds.size())
		{
			auto dsI = m_Shaders[DX12_SHADER_DOMAIN].find(ds);
			assert(dsI != m_Shaders[DX12_SHADER_DOMAIN].end());

			newPSO.DS.BytecodeLength = dsI->second.shader->GetBufferSize();
			newPSO.DS.pShaderBytecode = dsI->second.shader->GetBufferPointer();
		}

		newPSO.SampleMask = UINT_MAX;
		newPSO.SampleDesc.Count = 1;
		newPSO.SampleDesc.Quality = 0;

		ThrowIfFailed(m_Device->CreateGraphicsPipelineState(&newPSO, IID_PPV_ARGS(m_PSOs[keyName].GetAddressOf())));
		psoI = m_PSOs.find(keyName);
	}

	if (cmd)
	{
		cmd->SetPipelineState(psoI->second.Get());
		cmd->SetGraphicsRootSignature(rootSigI->second.Get());
	}
}

void PSOController::AddShader(const std::string& shaderName, DX12_SHADER_TYPE type,
	const std::wstring& filename, const D3D_SHADER_MACRO* defines,
	const std::string& entrypoint)
{
	string target;

	switch (type)
	{
	case DX12_SHADER_VERTEX:
		target = "vs_5_1";
		break;
	case DX12_SHADER_PIXEL:
		target = "ps_5_1";
		break;
	case DX12_SHADER_GEOMETRY:
		target = "gs_5_1";
		break;
	case DX12_SHADER_HULL:
		target = "hs_5_1";
		break;
	case DX12_SHADER_DOMAIN:
		target = "ds_5_1";
		break;
	case DX12_SHADER_TYPE_COUNT:
	default:
		assert(false);
		break;
	}

	auto iter = m_Shaders[type].find(shaderName);
	assert(iter == m_Shaders[type].end());

	UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	HRESULT hr = S_OK;

	ComPtr<ID3DBlob> byteCode = nullptr;
	ComPtr<ID3DBlob> errors;
	hr = D3DCompileFromFile(filename.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		entrypoint.c_str(), target.c_str(), compileFlags, 0, &byteCode, &errors);

	if (errors != nullptr)
		OutputDebugStringA((char*)errors->GetBufferPointer());

	ThrowIfFailed(hr);

	m_Shaders[type].insert({ { shaderName }, { type, byteCode } });
}

void PSOController::AddInputLayout(const std::string& name, const std::vector<D3D12_INPUT_ELEMENT_DESC>& inputLayout)
{
	auto iter = m_InputLayouts.find(name);
	assert(iter == m_InputLayouts.end());

	m_InputLayouts[name] = inputLayout;
}

void PSOController::AddBlend(const std::string& name, const D3D12_BLEND_DESC& blend)
{
	auto iter = m_Blends.find(name);
	assert(iter == m_Blends.end());

	m_Blends[name] = blend;
}

void PSOController::AddDepthStencil(const std::string& name, const D3D12_DEPTH_STENCIL_DESC& depthStencil)
{
	auto iter = m_DepthStencils.find(name);
	assert(iter == m_DepthStencils.end());

	m_DepthStencils[name] = depthStencil;
}

void PSOController::AddRasterizer(const std::string& name, const D3D12_RASTERIZER_DESC& rasterizer)
{
	auto iter = m_Rasterizers.find(name);
	assert(iter == m_Rasterizers.end());

	m_Rasterizers[name] = rasterizer;
}

void PSOController::AddRootSignature(const std::string& name, const D3D12_ROOT_SIGNATURE_DESC& rootSignature)
{
	auto iter = m_RootSignature.find(name);
	assert(iter == m_RootSignature.end());
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> error = nullptr;

	HRESULT hr = D3D12SerializeRootSignature(&rootSignature, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), error.GetAddressOf());

	if (error != nullptr)
	{
		::OutputDebugStringA((char*)error->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(m_Device->CreateRootSignature(0, serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(), IID_PPV_ARGS(m_RootSignature[name].GetAddressOf())));
}