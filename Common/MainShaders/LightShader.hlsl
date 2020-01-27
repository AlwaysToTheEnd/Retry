#include "Header.hlsli"

struct LightVSIn
{
    float4 PosnAngle : POSITIONANGLE;
    float3 Color : LIGHTCOLOR;
    float3 FallOffnPower : FALLOFFnPOWER;
    float3 Dir : LIGHTDIR;
};

struct PatchTess
{
    float Edges[4] : SV_TessFactor;
    float Inside[2] : SV_InsideTessFactor;
};

struct LightDSOutput
{
    float4 Position : SV_POSITION;
    float2 cpPos : CSPOS;
    float3 Color : LIGHTCOLOR;
    float3 Normal : DSNORMAL;
    nointerpolation float3 Dir : LIGHTDIR;
};

int LightType : register(c1);
/////////////////////////////////////////////////////////////////////////////////

LightVSIn VS(LightVSIn In)
{
    return In;
}

///////////////////////////////////////////////////////////////////////////////////

PatchTess LightConstantHS()
{
    PatchTess Output;
    float tessFactor = 8;
    
    switch (LightType)
    {
        case 0:
            tessFactor = 1;
            break;
        case 1:
            tessFactor = 18;
            break;
        case 2:
            tessFactor = 12;
            break;
    }
    
    Output.Edges[0] = Output.Edges[1] = Output.Edges[2] = Output.Edges[3] = tessFactor;
    Output.Inside[0] = Output.Inside[1] = tessFactor;

    return Output;
}

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_ccw")]
[outputcontrolpoints(4)]
[patchconstantfunc("LightConstantHS")]
LightVSIn HS(InputPatch<LightVSIn, 1> patch)
{
    return patch[0];
}

////////////////////////////////////////////////////////////////////////////////////////////////////

[domain("quad")]
LightDSOutput DS(PatchTess input, float2 UV : SV_DomainLocation, const OutputPatch<LightVSIn, 4> quad)
{
    LightDSOutput Output;
    Output.Normal = float3(0, 1, 0);
    Output.cpPos = UV * 2.0 - 1.0;
    Output.Color = quad[0].Color;
    Output.Dir = quad[0].Dir;

    switch (LightType)
    {
        case 0:
            Output.Position = float4(Output.cpPos, 0.999999f, 1);
            break;
        case 1:
            break;
        case 2:
            break;
    }

    return Output;
}

/////////////////////////////////////////////////////////////////////////////////////////////

float4 CalcDirectional(float3 position, SurfaceData surface, float3 color, float3 dir)
{
    float NDotL = dot(-dir, surface.Normal);
    float3 finalColor = color * saturate(NDotL);
   
    float3 ToEye = gEyePosW - position;
    ToEye = normalize(ToEye);
    float3 HalfWay = normalize(ToEye + -dir);
    float NDotH = saturate(dot(HalfWay, surface.Normal));
    finalColor += color * pow(NDotH, surface.SpecPower);

    return float4(finalColor * surface.Color.rgb, 1);
}

float4 PS(LightDSOutput In) : SV_Target
{    
    SurfaceData gbd = UnpackGBufferL(In.Position.xy);
    In.Dir = normalize(In.Dir);
    float3 position = CalcWorldPos(In.Position.xy, gbd.LinearDepth);
    float3 finalColor = float3(0, 0, 0);
    
    if (LightType == 0)
    {
        finalColor = (gbd.Color * gAmbientLight).rgb;
        finalColor += CalcDirectional(position, gbd, In.Color, In.Dir);
    }

    return float4(finalColor, 1);
}