#include "InOutStructs.hlsli"

struct MaterialData
{
    float4  DiffuseAlbedo;
    float3  Specular;
    float   SpecularExponent;
    float3  Emissive;
    float   Pad0;
};

struct SurfaceData
{
    float4  Color;
    float3  Normal;
    float   LinearDepth;
    float   SpecPower;
};

struct LightData
{
    float4 PosnAngle;
    float4 Color;
    float4 FallOffnPower;
    float4 Dir;
};

Texture2D gDepthTexture                         : register(t0);
Texture2D gNormalTexture                        : register(t1);
Texture2D gSpecPowerTexture                     : register(t2);
Texture2D gColorTexture                         : register(t3);
StructuredBuffer<int> gObjectIDTexture          : register(t4);
Texture2D gMainTexture[MAXTEXTURE]              : register(t5);
StructuredBuffer<MaterialData> gMaterialData    : register(t0, space1);

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
    float2      gMousePos;
    float2      gPad0;
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

PSOut GetPSOut(float4 baseColor, float3 normal, float specPower, int id)
{
    PSOut result;
    
    result.color = baseColor;
    result.normal = float4(normal * 0.5 + 0.5 , 1);
    result.specPow = float4(specPower, 0, 0, 1);
    result.objectID = int4(1,1,1,1);
    
    return result;
}

float3 CalcWorldPos(float2 csPos, float depth)
{
    float4 position;

    position.xy = csPos.xy * float2(1/gProj._11, 1/gProj._22) * depth;
    position.z = depth;
    position.w = 1.0;
	
    return mul(position, gInvView).xyz;
}


SurfaceData UnpackGBufferL(int2 location)
{
    SurfaceData result;
    
    int3 location3= int3(location, 0);
    float depth = gDepthTexture.Load(location3).x;
    result.Normal = gNormalTexture.Load(location3).xyz;
    
    result.LinearDepth = gProj._43 / (depth - gProj._33);
    result.Color = gColorTexture.Load(location3);
    result.Normal = normalize(result.Normal * 2.0f - 1.0);
    result.SpecPower = gSpecPowerTexture.Load(location3).x;
    
    return result;  
}