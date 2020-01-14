#include "PSOController.h"
#include "d3dUtil.h"
#include <D3Dcompiler.h>
#pragma comment(lib,"d3dcompiler.lib")


PSOController::PSOController(ID3D12Device* device)
	:m_Device(device)
{
}

PSOController::~PSOController()
{

}

ID3D12PipelineState* PSOController::GetPSO(
	const std::string& input, const std::string& rootSig, 
	const std::string& rasterizer, const std::string& blend, const std::string& depthStencil, 
	const std::string& vs, const std::string& ps, 
	const std::string& gs, const std::string& hs, const std::string& ds)
{
	ID3D12PipelineState* result = nullptr;

	std::string keyName =	input + ',' +	
							rootSig + ',' + 
							rasterizer + ',' + 
							blend + ',' + 
							depthStencil + ',' +
							vs + ',' + ps + ','+ gs + ',' + hs + ',' + ds;


	auto iter = m_PSOs.find(keyName);

	if (iter == m_PSOs.end())
	{
		result = CreatePSO(keyName, input, rootSig, rasterizer, blend, depthStencil, vs, ps, gs, hs, ds);
	}
	else
	{
		result = iter->second.Get();
	}

	return result;
}

void PSOController::AddShader(const std::string& shaderName, DX12_SHADER_TYPE type, const std::wstring& filename, const D3D_SHADER_MACRO* defines, const std::string& entrypoint, const std::string& target)
{
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

ID3D12PipelineState* PSOController::CreatePSO(const std::string& name,
	const std::string& input, const std::string& rootSig, const std::string& rasterizer, 
	const std::string& blend, const std::string& depthStencil, 
	const std::string& vs, const std::string& ps, 
	const std::string& gs, const std::string& hs, const std::string& ds)
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC newPSO = {};

	auto inputI = m_InputLayouts.find(input);
	auto rooSigI = m_RootSignature.find(rootSig);
	auto rasterI = m_Rasterizers.find(rasterizer);

	auto blendI = m_Blends.find(blend);
	auto depthStencilI = m_DepthStencils.find(depthStencil);

	auto vsI = m_Shaders[DX12_SHADER_VERTEX].find(vs);
	auto psI = m_Shaders[DX12_SHADER_PIXEL].find(ps);

	if (gs.size())
	{

	}

	if (hs.size())
	{

	}

	if (ds.size())
	{

	}

	m_Device->CreateGraphicsPipelineState(&newPSO, IID_PPV_ARGS(m_PSOs[name].GetAddressOf()));
}
