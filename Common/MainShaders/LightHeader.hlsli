#include "InOutStructs.hlsli"
#include "BaseRoot.hlsli"

struct SurfaceData
{
    float4 Color;
    float3 Normal;
    float LinearDepth;
    float SpecPower;
    int  ObjectID;
};

struct LightData
{
    float4 PosnAngle;
    float4 Color;
    float4 FallOffnPower;
    float4 Dir;
};

Texture2D<float4> gColorTexture : register(t0);
Texture2D<float3> gNormalTexture : register(t1);
Texture2D<float> gSpecPowerTexture : register(t2);
Texture2D<int> gObjectIDTexture : register(t3);
Texture2D<float> gDepthTexture : register(t4);

SurfaceData UnpackGBuffer(float2 UV)
{
    SurfaceData result;

    result.Color = gColorTexture.Sample(gsamPointWrap, UV.xy);
    result.Normal = gNormalTexture.Sample(gsamPointWrap, UV.xy).xyz;
    result.Normal = normalize(result.Normal * 2.0 - 1.0);
    result.SpecPower = gSpecPowerTexture.Sample(gsamPointWrap, UV.xy).x;
    result.LinearDepth = gDepthTexture.Sample(gsamPointWrap, UV.xy).x;
    result.LinearDepth = gProj._43 / (result.LinearDepth - gProj._33);

    return result;
}

SurfaceData UnpackGBufferL(int2 location)
{
    SurfaceData result;
    
    int3 location3 = int3(location, 0);
    float depth = gDepthTexture.Load(location3).x;
    result.Normal = gNormalTexture.Load(location3).xyz;
    
    result.LinearDepth = gProj._43 / (depth - gProj._33);
    result.Color = gColorTexture.Load(location3);
    result.Normal = normalize(result.Normal * 2.0f - 1.0);
    result.SpecPower = gSpecPowerTexture.Load(location3);
    result.ObjectID = gObjectIDTexture.Load(location3);
    
    return result;
}

float3 CalcWorldPos(float2 csPos, float depth)
{
    float4 position;

    position.xy = csPos.xy * float2(1 / gProj._11, 1 / gProj._22) * depth;
    position.z = depth;
    position.w = 1.0;
	
    return mul(position, gInvView).xyz;
}