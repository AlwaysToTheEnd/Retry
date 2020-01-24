#include "InOutStructs.hlsli"

struct MaterialData
{
    float4  DriffuseAlbedo;
    float3  FresnelR0;
    float   Roughness;
};

struct Light
{
    float4x4        shadowMatrix;
    float3          lightColor;
    float           falloffStart;
    float3          direction;
    float           falloffEnd;
    float3          position;
    float           spotPower;
    int             type;
    unsigned int    pad0;
    unsigned int    pad1;
    unsigned int    pad2;
};

Texture2D gMainTexture[MAXTEXTURE]              : register(t0);
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
    Light       gLight[15];
};

float4 GetTexel(uint textureIndex, float2 uv)
{
    float4 result = float4(0, 0, 0, 0);
    
    switch (gSamplerIndex)
    {
        case 0:
            result = gMainTexture[textureIndex].Sample(gsamPointWrap, uv);
            break;
        case 1:
            result = gMainTexture[textureIndex].Sample(gsamPointClamp, uv);
            break;
        case 2:
            result = gMainTexture[textureIndex].Sample(gsamLinearWrap, uv);
            break;
        case 3:
            result = gMainTexture[textureIndex].Sample(gsamLinearClamp, uv);
            break;
        case 4:
            result = gMainTexture[textureIndex].Sample(gsamAnisotropicWrap, uv);
            break;
        case 5:
            result = gMainTexture[textureIndex].Sample(gsamAnisotropicClamp, uv);
            break;
    }
    
    return result;
}

VertexOut VertexBaseWork(VertexIn vin, float4x4 worldMat, int normalMapIndex)
{
    VertexOut vout;
    
    vout.TexC = vin.TexC;
    
    vout.PosH = float4(vin.PosL, 1.0f);
    vout.PosH = mul(vout.PosH, worldMat);
    vout.ViewDir = normalize(vout.PosH.xyz - gEyePosW);
    
    vout.PosH = mul(vout.PosH, gViewProj);

    if(normalMapIndex < 0)
    {
        float3 worldNormal = normalize(mul(vin.NormalL, (float3x3) worldMat));
        vout.Diffuse = dot(-gLight[0].direction, worldNormal);
        vout.Reflection = reflect(gLight[0].direction, worldNormal);
    }
    
    return vout;
}