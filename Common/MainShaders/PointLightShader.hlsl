#include "Header.hlsli"

StructuredBuffer<LightData> gLightDatas : register(t1, space1);

float4 VS() : SV_Position
{
    return float4(0.0, 0.0, 0.0, 1.0);
}

struct HS_CONSTANT_DATA_OUTPUT
{
    float Edges[4] : SV_TessFactor;
    float Inside[2] : SV_InsideTessFactor;
    uint Index : LIGHTDATAINDEX;
};

HS_CONSTANT_DATA_OUTPUT PointLightConstantHS(uint PatchID : SV_PrimitiveID)
{
    HS_CONSTANT_DATA_OUTPUT Output;
    Output.Index = PatchID / 2;
	
    float tessFactor = 18.0;
    Output.Edges[0] = Output.Edges[1] = Output.Edges[2] = Output.Edges[3] = tessFactor;
    Output.Inside[0] = Output.Inside[1] = tessFactor;

    return Output;
}

struct HS_OUTPUT
{
    float3 HemiDir : POSITION;
};

static const float3 HemilDir[2] =
{
    float3(1.0, 1.0, 1.0),
	float3(-1.0, 1.0, -1.0)
};

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("PointLightConstantHS")]
HS_OUTPUT HS(uint PatchID : SV_PrimitiveID)
{
    HS_OUTPUT Output;

    Output.HemiDir = HemilDir[PatchID];

    return Output;
}

/////////////////////////////////////////////////////////////////////////////
// Domain Shader shader
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
	// Transform the UV's into clip-space
    float2 posClipSpace = UV.xy * 2.0 - 1.0;

	// Find the absulate maximum distance from the center
    float2 posClipSpaceAbs = abs(posClipSpace.xy);
    float maxLen = max(posClipSpaceAbs.x, posClipSpaceAbs.y);

	// Generate the final position in clip-space
    float3 normDir = normalize(float3(posClipSpace.xy, (maxLen - 1.0)) * quad[0].HemiDir);
    float4 posLS = float4(normDir.xyz, 1.0);
    posLS.xyz *= light.FallOffnPower.y;
    posLS.xyz += light.PosnAngle.xyz;
    
	// Transform all the way to projected space
    DS_OUTPUT Output;
    Output.Position = mul(posLS, gViewProj);
    Output.cpPos = Output.Position.xy / Output.Position.w;
    Output.Index = input.Index;

    return Output;
}

/////////////////////////////////////////////////////////////////////////////
// Pixel shader
/////////////////////////////////////////////////////////////////////////////

float3 CalcPoint(float3 position, SurfaceData surface, LightData light)
{
    float3 ToLight = light.PosnAngle.xyz - position;
    //float3 ToEye = gEyePosW - position;
    float DistToLight = length(ToLight);
   
   // Phong diffuse
    ToLight /= DistToLight; // Normalize
    float NDotL = saturate(dot(-ToLight, surface.Normal));
    float3 finalColor = surface.Color.rgb * NDotL;
   
   //// Blinn specular
   // ToEye = normalize(ToEye);
   // float3 HalfWay = normalize(ToEye + ToLight);
   // float NDotH = saturate(dot(HalfWay, surface.Normal));
   // finalColor += pow(NDotH, surface.SpecPower) * 0.5f;
    
    float attn = 1 - saturate(DistToLight / light.FallOffnPower.y);
    finalColor += light.Color.rgb * attn;
   
    return finalColor;
}

float4 PS(DS_OUTPUT In) : SV_TARGET
{
    SurfaceData gbd = UnpackGBufferL(In.Position.xy);
    LightData light = gLightDatas[In.Index];

	// Reconstruct the world position
    float3 position = CalcWorldPos(In.cpPos.xy, gbd.LinearDepth);

	// Calculate the light contribution
    float3 finalColor = CalcPoint(position, gbd, light);

	// return the final color
    return float4(finalColor, 1.0);
}