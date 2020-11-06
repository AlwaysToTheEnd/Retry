#include "InOutStructs.hlsli"
#include "BaseRoot.hlsli"

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

PSOut GetPSOut(float4 baseColor, float3 normal, float specPower, int objectID)
{
    PSOut result;
    
    result.color = baseColor;
    result.normal = (normal * 0.5).xyz + float3(0.5f, 0.5f, 0.5f);
    result.specPow = specPower;
    result.objectID = objectID;
    
    return result;
}