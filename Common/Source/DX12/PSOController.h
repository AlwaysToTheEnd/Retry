#pragma once
#include <d3d12.h>
#include <string>
#include <unordered_map>
#include <wrl.h>
#include "DX12RenderClasses.h"

using Microsoft::WRL::ComPtr;

class PSOController
{
	const char* baseAttributeName = "_`b";
	const char* shaderVersion = "_5_1";
public:
	PSOController(ID3D12Device* device);
	~PSOController();
	
	void InitBase_Raster_Blend_Depth();

	void SetPSOToCommnadList(ID3D12GraphicsCommandList* cmd, const std::string& rootSig, const std::string& cs);
	void SetPSOToCommnadList(ID3D12GraphicsCommandList* cmd,
		const std::vector<DXGI_FORMAT>& rtvFormats, DXGI_FORMAT dsvFormat, D3D12_PRIMITIVE_TOPOLOGY_TYPE primitive,
		const std::string& input, const std::string& rootSig, 
		const std::string& rasterizer, const std::string& blend, const std::string& depthStencil, 
		const std::string& vs="", const std::string& ps="", 
		const std::string& gs="", const std::string& hs="", const std::string& ds = "");

	void AddShader(const std::string& shaderName, DX12_SHADER_TYPE type, const std::wstring& filename, const D3D_SHADER_MACRO* defines, const std::string& entrypoint);
	void AddInputLayout(const std::string& name, const std::vector<D3D12_INPUT_ELEMENT_DESC>& inputLayout);
	void AddBlend(const std::string& name, const D3D12_BLEND_DESC& blend);
	void AddDepthStencil(const std::string& name, const D3D12_DEPTH_STENCIL_DESC& depthStencil);
	void AddRasterizer(const std::string& name, const D3D12_RASTERIZER_DESC& rasterizer);
	void AddRootSignature(const std::string& name, const D3D12_ROOT_SIGNATURE_DESC& rootSignature);
	void AddCommandSignature(const std::string& name, const std::string& rootSigName, const D3D12_COMMAND_SIGNATURE_DESC& commandSigDesc);

	ID3D12CommandSignature* GetCommandSignature(const std::string& name);

private:
	ID3D12Device* m_Device;
	
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>>			m_PSOs;
	std::unordered_map<std::string, D3D12_BLEND_DESC>						m_Blends;
	std::unordered_map<std::string, D3D12_DEPTH_STENCIL_DESC>				m_DepthStencils;
	std::unordered_map<std::string, D3D12_RASTERIZER_DESC>					m_Rasterizers;
	std::unordered_map<std::string, ComPtr<ID3DBlob>>						m_Shaders[DX12_SHADER_TYPE_COUNT];//
	std::unordered_map<std::string, ComPtr<ID3D12RootSignature>>			m_RootSignature;
	std::unordered_map<std::string, ComPtr<ID3D12CommandSignature>>			m_CommandSignature;
	std::unordered_map<std::string, std::vector<D3D12_INPUT_ELEMENT_DESC>>	m_InputLayouts;
};