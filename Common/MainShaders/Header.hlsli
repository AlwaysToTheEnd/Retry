#include "InOutStructs.hlsli"

struct MaterialData
{
    float4  DriffuseAlbedo;
    float3  FresnelR0;
    float   Roughness;
};

Texture2D gDepthTexture                         : register(t0);
Texture2D gNormalTexture                        : register(t1);
Texture2D gSpecPowerTexture                     : register(t2);
Texture2D gColorTexture                         : register(t3);
Texture2D gMainTexture[MAXTEXTURE]              : register(t4);
StructuredBuffer<MaterialData>  gMaterialData   : register(t0, space1);

SamplerState            gsamPointWrap           : register(s0);
SamplerState            gsamPointClamp          : register(s1);
SamplerState            gsamLinearWrap          : register(s2);
SamplerState            gsamLinearClamp         : register(s3);
SamplerState            gsamAnisotropicWrap     : register(s4);
SamplerState            gsamAnisotropicClamp    : register(s5);
SamplerComparisonState  gsamShadow              : register(s6);

cbuffer cbPass                                  : register(b0)
{
    float4x4    gView;
    float4x4    gInvView;
    float4x4    gProj;
    float4x4    gInvProj;
    float4x4    gViewProj;
    float4x4    gInvViewProj;
    float4x4    gRightViewProj;
    float4x4    gOrthoMatrix;
    float2      gRenderTargetSize;
    float2      gInvRenderTargetSize;
    float4      gAmbientLight;
    float3      gEyePosW;
    uint        gSamplerIndex;
};

float4 GetTexel(uint textureIndex, float2 uv)
{
    float4 result = float4(0, 0, 0, 0);
    
    switch (gSamplerIndex)
    {
        case 0: result = gMainTexture[textureIndex].Sample(gsamPointWrap, uv); break;
        case 1: result = gMainTexture[textureIndex].Sample(gsamPointClamp, uv); break;
        case 2: result = gMainTexture[textureIndex].Sample(gsamLinearWrap, uv); break;
        case 3: result = gMainTexture[textureIndex].Sample(gsamLinearClamp, uv); break;
        case 4: result = gMainTexture[textureIndex].Sample(gsamAnisotropicWrap, uv); break;
        case 5: result = gMainTexture[textureIndex].Sample(gsamAnisotropicClamp, uv); break;
    }
    
    return result;
}

VertexOut VertexBaseWork(VertexIn vin, float4x4 worldMat)
{
    VertexOut vout;
    
    vout.TexC = vin.TexC;
    vout.PosH = mul(float4(vin.PosL, 1.0f), worldMat);
    vout.PosH = mul(vout.PosH, gViewProj);
    vout.Normal = mul(vin.NormalL, (float3x3) worldMat);
    
    return vout;
}

PSOut GetPSOut(float4 baseColor, float3 normal, float specPower)
{
    PSOut result;
    
    result.color = baseColor;
    result.normal = float4(normal * 0.5 + 0.5 , 0);
    result.specPow = float4(specPower, 0, 0, 0);
    
    return result;
}