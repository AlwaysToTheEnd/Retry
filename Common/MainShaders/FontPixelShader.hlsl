#include "InOutStructs.hlsli"

Texture2D		Texture : register(t0);
SamplerState	TextureSampler : register(s0);
cbuffer cbSettings : register(b1)
{
    int gObjectID;
}

cbuffer cbSettings2 : register(b2)
{
    int gObjectID2;
}

UIPSOut main(float4 color : COLOR0, float2 texCoord : TEXCOORD0)
{
    UIPSOut result;
    result.color = Texture.Sample(TextureSampler, texCoord);
    result.objectID = gObjectID;
    
    return result;
}