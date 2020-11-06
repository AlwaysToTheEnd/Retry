#include "LightHeader.hlsli"

StructuredBuffer<LightData> gLightDatas : register(t5);

float4 VS() : SV_Position
{
    return float4(0.0, 0.0, 0.0, 1.0);
}
/////////////////////////////////////////////////////////////////////////////////////////

struct HS_CONSTANT_DATA_OUTPUT
{
    float Edges[4] : SV_TessFactor;
    float Inside[2] : SV_InsideTessFactor;
    nointerpolation uint Index : LIGHTDATAINDEX;
};

HS_CONSTANT_DATA_OUTPUT PointLightConstantHS(uint PatchID : SV_PrimitiveID)
{
    HS_CONSTANT_DATA_OUTPUT Output;
    Output.Index = PatchID/(uint)2;
	
    float tessFactor = 18.0;
    Output.Edges[0] = Output.Edges[1] = Output.Edges[2] = Output.Edges[3] = tessFactor;
    Output.Inside[0] = Output.Inside[1] = tessFactor;

    return Output;
}

struct HS_OUTPUT
{
    float3 HemiDir : POSITION;
};

static const float3 HemilDir[2] = {
    float3(1.0, 1.0,1.0),
    float3(-1.0, 1.0, -1.0)
};

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_ccw")]
[outputcontrolpoints(4)]
[patchconstantfunc("PointLightConstantHS")]
HS_OUTPUT HS(uint PatchID : SV_PrimitiveID)
{
    HS_OUTPUT Output;
    Output.HemiDir = HemilDir[PatchID%2];
    return Output;
}

/////////////////////////////////////////////////////////////////////////////
struct DS_OUTPUT
{
    float4 Position : SV_POSITION;
    float2 cpPos : TEXCOORD0;
    nointerpolation uint Index : LIGHTDATAINDEX;
};

[domain("quad")]
DS_OUTPUT DS(HS_CONSTANT_DATA_OUTPUT input, float2 UV : SV_DomainLocation, const OutputPatch<HS_OUTPUT, 4> quad)
{
    LightData light = gLightDatas[input.Index];

    float2 posClipSpace = UV.xy * 2.0 - 1.0;
    float2 posClipSpaceAbs = abs(posClipSpace.xy);
    float maxLen = max(posClipSpaceAbs.x, posClipSpaceAbs.y);

    float3 CenterPos = mul(float4(light.PosnAngle.xyz, 1), gView).xyz;
    float3 normDir = normalize(float3(posClipSpace.xy, (maxLen - 1.0)) * quad[0].HemiDir) * light.FallOffnPower.y;
    float4 posLS = float4(CenterPos + normDir.xyz, 1.0);

    DS_OUTPUT Output;
    Output.Position = mul(posLS, gProj);

    Output.cpPos = Output.Position.xy / Output.Position.w;
    Output.Index = input.Index;

    return Output;
}

////////////////////////////////////////////////////////////////////////////////////
float3 CalcPoint(float3 position, float3 surfaceRGB, LightData light)
{
    float3 toLight = light.PosnAngle.xyz - position;
    float distToLight = length(toLight);
   
    if (distToLight > light.FallOffnPower.y)
    {
        clip(-1);
    }
   
    float dstToLightNorm = 1.0 - saturate(distToLight / light.FallOffnPower.y);
    float attn = dstToLightNorm * dstToLightNorm;
    float3 finalColor = surfaceRGB + light.Color.rgb * attn;
   
    return finalColor;
}

float4 PS(DS_OUTPUT In) : SV_Target0
{
    SurfaceData gbd = UnpackGBufferL(In.Position.xy);
    LightData light = gLightDatas[In.Index];
    
    float3 position = CalcWorldPos(In.cpPos.xy, gbd.LinearDepth);
    float3 finalColor = CalcPoint(position, gbd.Color.rgb, light);
    
    return float4(finalColor, 1.0);
}