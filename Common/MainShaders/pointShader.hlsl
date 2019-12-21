
struct ObjectData
{
	float4x4	World;
	int			TextureIndex;
	int			pad0;
	int			pad1;
	int			pad2;
};

StructuredBuffer<ObjectData> gObjectData : register(t0, space1);
Texture2D gMainTexture[MAXTEXTURE] : register(t0);

SamplerState gsamPointWrap : register(s0);
SamplerState gsamPointClamp : register(s1);
SamplerState gsamLinearWrap : register(s2);
SamplerState gsamLinearClamp : register(s3);
SamplerState gsamAnisotropicWrap : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);
SamplerComparisonState gsamShadow : register(s6);

cbuffer cbPass : register(b0)
{
	float4x4 gView;
	float4x4 gInvView;
	float4x4 gProj;
	float4x4 gInvProj;
	float4x4 gViewProj;
	float4x4 gInvViewProj;
	float4x4 gRightViewProj;
	float4x4 gShadowMapMatrix;
	float3 gEyePosW;
	float cbPerObjectPad1;
	float2 gRenderTargetSize;
	float2 gInvRenderTargetSize;
	float4 gAmbientLight;
};

struct VertexIn
{
	uint	type : TYPE;
	uint	cbIndex : CBINDEX;
	float3	size : SIZE;
	float4	color : COLOR;
};

struct VertexOut
{
	float4	PosH : SV_POSITION;
	float4	Color : COLOR0;
	float3	Normal : NORMAL;
	nointerpolation int TexIndex : MATNDEX;
};

VertexIn VS(VertexIn vin)
{
	return vin;
}

//enum RENDER_TYPE
//{
//	RENDER_NONE,
//	RENDER_MESH,
//	RENDER_BOX,
//	RENDER_PLANE,
//	RENDER_TEX_PLANE,
//};

[maxvertexcount(16)]
void GS(point VertexIn input[1], inout TriangleStream<VertexOut> output)
{
	switch (input[0].type)
	{
		switch (it.type)
		{
		case 2:
			break;
		case 3:
			break;
		case 4:
			break;
		}
	}

	output.RestartStrip();
}

float4 PS(VertexOut pin) : SV_Target
{
	float4 litColor = float4(0,0,0,1);
	int index = gInstanceData[MaterialIndex].DiffuseMapIndex;

	if (index >= 0)
	{
		litColor = gMainTexture[index].Sample(gsamPointWrap, pin.TexC);
	}

	litColor.a = 1.0f;
	return litColor;
}