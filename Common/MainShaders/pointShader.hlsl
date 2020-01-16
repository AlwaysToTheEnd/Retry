#include "Header.hlsli"

struct ObjectData
{
	float4x4	World;
	int			TextureIndex;
	int			pad0;
	int			pad1;
	int			pad2;
};

StructuredBuffer<ObjectData> gObjectData : register(t0, space1);

struct VertexIn
{
	uint	type : MESHTYPE;
	uint	cbIndex : CBINDEX;
	float3	size : MESHSIZE;
	float4	color : MESHCOLOR;
};

struct VertexOut
{
	float4	posH : SV_POSITION;
	float4	color : COLOR0;
	nointerpolation int texIndex : MATNDEX;
};

VertexIn VS(VertexIn vin)
{
	return vin;
}

void CreateBox(uint cbIndex, float3 size, float4 color, inout TriangleStream<VertexOut> output)
{
	VertexOut vertices[8];
	// 1  3  7  5
	// |\ |  | /|
	// 0 \2  6/ 4
	vertices[0].posH = float4(-size.x, -size.y, -size.z, 1.0f);
	vertices[1].posH = float4(-size.x, size.y, -size.z, 1.0f);
	vertices[2].posH = float4(size.x, -size.y, -size.z, 1.0f);
	vertices[3].posH = float4(size.x, size.y, -size.z, 1.0f);
	vertices[4].posH = float4(size.x, -size.y, size.z, 1.0f);
	vertices[5].posH = float4(size.x, size.y, size.z, 1.0f);
	vertices[6].posH = float4(-size.x, -size.y, size.z, 1.0f);
	vertices[7].posH = float4(-size.x, size.y, size.z, 1.0f);

	[unroll]
	for (int i = 0; i < 8; i++)
	{
		vertices[i].color = color;
		vertices[i].texIndex = gObjectData[cbIndex].TextureIndex;
		vertices[i].posH = mul(vertices[i].posH, gObjectData[cbIndex].World);
		vertices[i].posH = mul(vertices[i].posH, gViewProj);
	}

	output.Append(vertices[0]);
	output.Append(vertices[1]);
	output.Append(vertices[2]);
	output.Append(vertices[3]);
	output.Append(vertices[4]);
	output.Append(vertices[5]);
	output.Append(vertices[6]);
	output.Append(vertices[7]);
	output.Append(vertices[0]);
	output.Append(vertices[1]);
	output.RestartStrip();

	output.Append(vertices[3]);
	output.Append(vertices[1]);
	output.Append(vertices[5]);
	output.Append(vertices[7]);
	output.RestartStrip();

	output.Append(vertices[0]);
	output.Append(vertices[2]);
	output.Append(vertices[6]);
	output.Append(vertices[4]);
	output.RestartStrip();
}

void CreatePlane(uint cbIndex, float2 size, float4 color, inout TriangleStream<VertexOut> output)
{
	VertexOut vertices[4];

	// 1  3
	// |\ |
	// 0 \2

	vertices[0].posH = float4(-size.x, -size.y, 0.0f, 1.0f);
	vertices[1].posH = float4(-size.x, size.y, 0.0f, 1.0f);
	vertices[2].posH = float4(size.x, -size.y, 0.0f, 1.0f);
	vertices[3].posH = float4(size.x, size.y, 0.0f, 1.0f);

	[unroll]
	for (int i = 0; i < 4; i++)
	{
		vertices[i].texIndex = gObjectData[cbIndex].TextureIndex;
		vertices[i].posH = mul(vertices[i].posH, gObjectData[cbIndex].World);
		vertices[i].posH = mul(vertices[i].posH, gViewProj);
	}

	if (gObjectData[cbIndex].TextureIndex > -1)
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
			vertices[i].color = color;
		}
	}

	output.Append(vertices[0]);
	output.Append(vertices[2]);
	output.Append(vertices[1]);
	output.Append(vertices[3]);
}


void Create2DPlane(uint cbIndex, float2 size, float4 color, inout TriangleStream<VertexOut> output)
{
    VertexOut vertices[4];

	// 1  3
	// |\ |
	// 0 \2

    vertices[0].posH = float4(-size.x, -size.y, 0.0f, 1.0f);
    vertices[1].posH = float4(-size.x, size.y, 0.0f, 1.0f);
    vertices[2].posH = float4(size.x, -size.y, 0.0f, 1.0f);
    vertices[3].posH = float4(size.x, size.y, 0.0f, 1.0f);

	[unroll]
    for (int i = 0; i < 4; i++)
    {
        vertices[i].texIndex = gObjectData[cbIndex].TextureIndex;
        vertices[i].posH = mul(vertices[i].posH, gObjectData[cbIndex].World);
        vertices[i].posH = mul(vertices[i].posH, gOrthoMatrix);
    }

    if (gObjectData[cbIndex].TextureIndex > -1)
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
            vertices[i].color = color;
        }
    }

    output.Append(vertices[0]);
    output.Append(vertices[2]);
    output.Append(vertices[1]);
    output.Append(vertices[3]);
}
//enum RENDER_TYPE
//{
//  RENDER_NONE,
//	RENDER_MESH,
//	RENDER_DYNAMIC,
//	RENDER_SKIN,
//	RENDER_BOX,
//	RENDER_PLANE,
//	RENDER_2DPLANE,
//	RENDER_UI,
//};

[maxvertexcount(18)]
void GS(point VertexIn input[1], inout TriangleStream<VertexOut> output)
{
	switch (input[0].type)
	{
	case 4: //BOX
		CreateBox(input[0].cbIndex, input[0].size, input[0].color, output);
		break;
	case 5: //PLANE
		CreatePlane(input[0].cbIndex, float2(input[0].size.x, input[0].size.y), input[0].color, output);
        break;
	case 6:
        Create2DPlane(input[0].cbIndex, float2(input[0].size.x, input[0].size.y), input[0].color, output);
		break;
	}
}

float4 PS(VertexOut pin) : SV_Target
{
	float4 resultColor= float4(0,0,0,1);

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