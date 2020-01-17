#include "DX12DrawSetHeightField.h"

void DX12DrawSetHeightField::Init(ID3D12Device* device, PSOController* psoCon, DXGI_FORMAT rtvFormat, DXGI_FORMAT dsvFormat, DX12TextureBuffer* textureBuffer, DX12IndexManagementBuffer<Material>* material, ID3D12Resource* mainPass)
{
}

void DX12DrawSetHeightField::Draw(ID3D12GraphicsCommandList* cmd, const PSOAttributeNames* custom)
{
}

void DX12DrawSetHeightField::ReserveRender(const RenderInfo& info)
{
}
