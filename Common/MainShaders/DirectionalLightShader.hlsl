#include "LightHeader.hlsli"

struct DLightVSOut
{
    uint Index : LIGHTDATAINDEX;
};

struct DLightGSOut
{
    float4 Position : SV_POSITION;
    nointerpolation uint Index : LIGHTDATAINDEX;
};

StructuredBuffer<LightData> gLightDatas : register(t5);
/////////////////////////////////////////////////////////////////////////////////

DLightVSOut VS(uint id : SV_VertexID)
{
    DLightVSOut result;
    result.Index = id;
    return result;
}

/////////////////////////////////////////////////////////////////////////////////////////////

[maxvertexcount(4)]
void GS(point DLightVSOut input[1], inout TriangleStream<DLightGSOut> output)
{
    DLightGSOut vertices[4];
    // 1  3
	// |\ |
	// 0 \2
    vertices[0].Position = float4(-1.0f, -1.0f, 0.99999f, 1.0f);
    vertices[1].Position = float4(-1.0f, 1.0f, 0.99999f, 1.0f);
    vertices[2].Position = float4(1.0f, -1.0f, 0.99999f, 1.0f);
    vertices[3].Position = float4(1.0f, 1.0f, 0.99999f, 1.0f);
   
	[unroll]
    for (int i = 0; i < 4; i++)
    {
        vertices[i].Index = input[0].Index;
    }

    output.Append(vertices[0]);
    output.Append(vertices[2]);
    output.Append(vertices[1]);
    output.Append(vertices[3]);
}

////////////////////////////////////////////////////////////////////////////////////////
float3 CalcDirectional(float3 position, SurfaceData surface, float3 color, float3 dir)
{
    float NDotL = dot(-dir, surface.Normal);
    float3 finalColor = color * saturate(NDotL);
   
    float3 ToEye = gEyePosW - position;
    ToEye = normalize(ToEye);
    float3 HalfWay = normalize(ToEye + -dir);
    float NDotH = saturate(dot(HalfWay, surface.Normal));
    finalColor += color * pow(NDotH, surface.SpecPower);

    return float3(finalColor * surface.Color.rgb);
}

float4 PS(DLightGSOut In) : SV_Target
{
    SurfaceData gbd = UnpackGBufferL(In.Position.xy);
    LightData light = gLightDatas[In.Index];
    light.Dir = normalize(light.Dir);
    float3 position = CalcWorldPos(In.Position.xy, gbd.LinearDepth);
    float3 finalColor = float3(0, 0, 0);
    
    finalColor = (gbd.Color * gAmbientLight).rgb;
    finalColor += CalcDirectional(position, gbd, light.Color.rgb, light.Dir.xyz);

    return float4(finalColor, 1);
}