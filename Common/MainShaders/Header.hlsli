struct VertexIn
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
    float2 TexC : TEXCOORD;
};

struct SkinnedVertexIn
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
    float2 TexC : TEXCOORD;
    float3 BoneWeights : WEIGHTS;
    uint4 BoneIndices : BONEINDICES;
};

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float2 TexC : TEXCOORD0;
    float3 Normal : NORMAL0;
};

struct POINTVertexIn
{
    uint type : MESHTYPE;
    uint cbIndex : CBINDEX;
    float3 size : MESHSIZE;
    float4 color : MESHCOLOR;
};

struct POINTVertexOut
{
    float4 posH : SV_POSITION;
    float4 color : COLOR0;
    nointerpolation int texIndex : MATNDEX;
};

struct UIVertexIn
{
    float4 color : UICOLOR;
    float3 pos : UIPOS;
    float2 size : UISIZE;
    int uiType : UITYPE;
    int textureIndex : UITEXTURE;
};

struct MaterialData
{
    float4 DriffuseAlbedo;
    float3 FresnelR0;
    float Roughness;
};

Texture2D gMainTexture[MAXTEXTURE]              : register(t0);
StructuredBuffer<MaterialData> gInstanceData    : register(t0, space1);

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
    float3      gEyePosW;
    float2      gRenderTargetSize;
    float2      gInvRenderTargetSize;
    float4      gAmbientLight;
    uint        gSamplerIndex;

	//Light gLights[MaxLights];
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