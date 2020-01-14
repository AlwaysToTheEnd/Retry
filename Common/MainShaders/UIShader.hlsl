#include "Header.hlsli"

cbuffer uiPass : register(b1)
{
    float4  gPanelTitleColor;
    int     gPanelTitleHeight;
    int     gPanelTitleTextHeight;
    int     gPanelCloseButtonHalfSize;
    int     gPanelComponentsInterval;
};

struct UIInfomation
{
    float4  color         : UICOLOR;
    float3  pos           : UIPOS;
    float2  size          : UISIZE;
    bool    isPanelUI     : UITYPE;
    int     textureIndex  : UITEXTURE;
};

struct VertexOut
{
    float4              posH     : SV_POSITION;
    float4              color    : COLOR0;
    nointerpolation int texIndex : MATNDEX;
};

UIInfomation VS(UIInfomation vin)
{
    return vin;
}

[maxvertexcount(10)]
void GS(point UIInfomation input[1], inout TriangleStream<VertexOut> output)
{
    if(input[0].isPanelUI)
    {
        
    }
    else
    {
        
    }
}

float4 PS(VertexOut pin) : SV_Target
{
    float4 resultColor = float4(0, 0, 0, 0);

    if (pin.texIndex > -1)
    {
        resultColor = GetTexel(pin.texIndex, float2(pin.color.x, pin.color.y));
    }
    else
    {
        resultColor = pin.color;
    }

    return resultColor;
}