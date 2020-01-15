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
    int     uiType        : UITYPE;
    int     textureIndex  : UITEXTURE;
};

struct VertexOut
{
    float4              posH     : SV_POSITION;
    float4              color    : COLOR0;
    nointerpolation int texIndex : TEXINDEX;
};

UIInfomation VS(UIInfomation vin)
{
    return vin;
}

void CreatePanel(UIInfomation vin, inout TriangleStream<VertexOut> output)
{
    VertexOut vertices[12];

    const float edgeSize = 5.0f;
    
    vertices[0].posH = float4(0, 0, 0.0f, 1.0f);
    vertices[1].posH = vertices[0].posH;
    
    vertices[2].posH = vertices[0].posH;
    vertices[2].posH.y += gPanelTitleHeight;
    
    vertices[3].posH = float4(vin.size.x, 0, 0.0f, 1.0f);
    vertices[4].posH = vertices[3].posH;
    
    vertices[5].posH = vertices[3].posH;
    
    vertices[4].posH.y += gPanelTitleHeight;
    
    vertices[0].posH.y += edgeSize;
    vertices[1].posH.x += edgeSize;
    
    vertices[3].posH.x -= edgeSize;
    vertices[5].posH.y += edgeSize;
    
    /////////////////////////////////////////////////////////////
    
    vertices[6].posH = float4(0, vin.size.y, 0.0f, 1.0f);
    vertices[7].posH = vertices[2].posH;
    vertices[8].posH = vertices[6].posH;
    
    vertices[9].posH = vertices[5].posH;
    vertices[10].posH = float4(vin.size.x, vin.size.y, 0.0f, 1.0f);
    vertices[11].posH = vertices[10].posH;
    
    vertices[6].posH.y -= edgeSize;
    vertices[8].posH.x += edgeSize;
    
    vertices[10].posH.x -= edgeSize;
    vertices[11].posH.y -= edgeSize;
    
    [unroll(6)]
    for (int i = 0; i < 6; i++)
    {
        vertices[i].texIndex = -1;
        vertices[i].color = gPanelTitleColor;
        vertices[i].posH = float4(vertices[i].posH.xyz + vin.pos, 1.0f);
        vertices[i].posH = mul(vertices[i].posH, gOrthoMatrix);
    }
    
    [unroll(6)]
    for (int j = 6; j < 12; j++)
    {
        vertices[j].texIndex = vin.textureIndex;
        vertices[j].color = vin.color;
        vertices[j].posH = float4(vertices[j].posH.xyz + vin.pos, 1.0f);
        vertices[j].posH = mul(vertices[j].posH, gOrthoMatrix);
    }
    
    //vertices[4].color.a = vertices[4].color.a / 2;
    
    output.Append(vertices[0]);
    output.Append(vertices[1]);
    output.Append(vertices[2]);
    output.Append(vertices[3]);
    output.Append(vertices[4]);
    output.Append(vertices[5]);
    output.RestartStrip();
    
    output.Append(vertices[6]);
    output.Append(vertices[7]);
    output.Append(vertices[8]);
    output.Append(vertices[9]);
    output.Append(vertices[10]);
    output.Append(vertices[11]);
}

void CreateUI(UIInfomation vin, inout TriangleStream<VertexOut> output)
{
    VertexOut vertices[4];

	// 1  3
	// |\ |
	// 0 \2

    vertices[0].posH = float4(-vin.size.x, vin.size.y, 0.0f, 1.0f);
    vertices[1].posH = float4(-vin.size.x, -vin.size.y, 0.0f, 1.0f);
    vertices[2].posH = float4(vin.size.x, vin.size.y, 0.0f, 1.0f);
    vertices[3].posH = float4(vin.size.x, -vin.size.y, 0.0f, 1.0f);

	[unroll(4)]
    for (int i = 0; i < 4; i++)
    {
        vertices[i].texIndex = vin.textureIndex;
        vertices[i].posH = mul(vertices[i].posH, vin.pos);
        vertices[i].posH = mul(vertices[i].posH, gOrthoMatrix);
    }

    if (vin.textureIndex > -1)
    {
        vertices[0].color = float4(0.0f, 1.0f, 0.0f, 0.0f);
        vertices[1].color = float4(0.0f, 0.0f, 0.0f, 0.0f);
        vertices[2].color = float4(1.0f, 1.0f, 0.0f, 0.0f);
        vertices[3].color = float4(1.0f, 0.0f, 0.0f, 0.0f);
    }
    else
    {
		[unroll]
        for (int i = 0; i < 4; i++)
        {
            vertices[i].color = vin.color;
        }
    }

    output.Append(vertices[0]);
    output.Append(vertices[1]);
    output.Append(vertices[2]);
    output.Append(vertices[3]);
}

[maxvertexcount(12)]
void GS(point UIInfomation input[1], inout TriangleStream<VertexOut> output)
{
    if(input[0].uiType==0)
    {
        CreatePanel(input[0], output);
    }
    else
    {
        CreateUI(input[0], output);
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