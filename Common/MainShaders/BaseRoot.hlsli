SamplerState gsamPointWrap : register(s0);
SamplerState gsamPointClamp : register(s1);
SamplerState gsamLinearWrap : register(s2);
SamplerState gsamLinearClamp : register(s3);
SamplerState gsamAnisotropicWrap : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);
SamplerComparisonState gsamShadow : register(s6);

cbuffer cbPass : register(b0)
{
    float4x4 gView;
    float4x4 gInvView;
    float4x4 gProj;
    float4x4 gInvProj;
    float4x4 gViewProj;
    float4x4 gInvViewProj;
    float4x4 gRightViewProj;
    float4x4 gOrthoMatrix;
    uint2 gRenderTargetSize;
    float2 gInvRenderTargetSize;
    float4 gAmbientLight;
    float3 gEyePosW;
    uint gSamplerIndex;
    float2 gMousePos;
    float2 gPad0;
};

struct MaterialData
{
    float4 DiffuseAlbedo;
    float3 Specular;
    float SpecularExponent;
    float3 Emissive;
    float Pad0;
};

StructuredBuffer<MaterialData> gMaterialData : register(t0, space1);
Texture2D gMainTexture[MAXTEXTURE] : register(t1, space1);