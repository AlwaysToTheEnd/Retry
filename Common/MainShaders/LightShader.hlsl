#include "Header.hlsli"

struct LightData
{
    float4 PosnAngle;
    float3 Color;
    int Type;
    float3 FallOffnPower;
    int Pad0;
    float3 dir;
    int Pad1;
};

struct PatchTess
{
    float Edges[4] : SV_TessFactor;
    float Inside[2] : SV_InsideTessFactor;
};

struct LightHSOutput
{
    float3 pos : POSITION;
    uint Index : LIGHTINDEX;
};

struct LightDSOutput
{
    float4 Position : SV_POSITION;
    float2 cpPos : CSPOS;
    float3 Normal : DSNORMAL;
    nointerpolation uint Index : LIGHTINDEX;
};

StructuredBuffer<LightData> LightDatas : register(t1, space1);
/////////////////////////////////////////////////////////////////////////////////

float4 VS() : SV_Position
{
    return float4(0, 0, 0, 1);
}

///////////////////////////////////////////////////////////////////////////////////

PatchTess LightConstantHS()
{
    PatchTess Output;
    float tessFactor = 8;
    
    //switch (LightDatas[PatchID].Type)
    //{
    //    case 0:
    //        tessFactor = 2;
    //        break;
    //    case 1:
    //        tessFactor = 18;
    //        break;
    //    case 2:
    //        tessFactor = 2;
    //        break;
    //}
    
    Output.Edges[0] = Output.Edges[1] = Output.Edges[2] = Output.Edges[3] = tessFactor;
    Output.Inside[0] = Output.Inside[1] = tessFactor;

    return Output;
}

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[maxtessfactor(32.0f)]
[patchconstantfunc("LightConstantHS")]
LightHSOutput HS(uint PatchID : SV_PrimitiveID)
{
    LightHSOutput Output;
    Output.pos = 0;
    Output.Index = PatchID;
    return Output;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

[domain("quad")]
LightDSOutput DS(PatchTess input, float2 UV : SV_DomainLocation, const OutputPatch<LightHSOutput, 4> quad)
{
    LightData lightData = LightDatas[quad[0].Index];
    LightDSOutput Output;
    Output.Index = quad[0].Index;
    Output.Normal = float3(0, 1, 0);
    Output.cpPos = UV.xy * 2.0 - 1.0;
    Output.Position = float4(Output.cpPos, 0.95, 1);
    switch (lightData.Type)
    {
        case 0:
            Output.cpPos = UV.xy * 2.0 - 1.0;
            Output.Position = float4(Output.cpPos, 0.95, 1);
            break;
        case 1:
            break;
        case 2:
            break;
    }

    return Output;
}

/////////////////////////////////////////////////////////////////////////////////////////////

float3 CalcAmbient(float3 normal, float3 color)
{
    float up = normal.y * 0.5 + 0.5;

    float3 ambient = float3(0.1, 0.2f, 0.1f) + up * float3(0.1, 0.2f, 0.2f);

    return ambient * color;
}

float3 CalcDirectional(float3 position, SurfaceData surface, LightData light)
{
    float NDotL = dot(-light.dir, surface.Normal);
    float3 finalColor = light.Color * saturate(NDotL);
   
    float3 ToEye = gEyePosW - position;
    ToEye = normalize(ToEye);
    float3 HalfWay = normalize(ToEye + -light.dir);
    float NDotH = saturate(dot(HalfWay, surface.Normal));
    finalColor += light.Color * pow(NDotH, surface.SpecPower);

    return finalColor * surface.Color.rgb;
}

float4 DirectionalLightRender(LightDSOutput In, SurfaceData gbd, LightData light)
{
    float3 position = CalcWorldPos(In.cpPos, gbd.LinearDepth);

    float3 finalColor = CalcAmbient(gbd.Normal, gbd.Color.rgb);

    finalColor += CalcDirectional(position, gbd, light);

    return float4(finalColor, 1.0);
}

float4 PS(LightDSOutput In) : SV_Target
{
    float4 result = float4(1, 1, 1, 1);
    
    SurfaceData gbd = UnpackGBuffer(In.cpPos);
    LightData lightData = LightDatas[In.Index];

    if (lightData.Type == 0)
    {
        result = DirectionalLightRender(In, gbd, lightData);
    }

    return float4(1, 1, 1, 1);
}