struct MaterialData
{
	float4 DriffuseAlbedo;
	float3 FresnelR0;
	float Roughness;
	float4x4 MatTransform;
};

StructuredBuffer<MaterialData> gInstanceData : register(t0, space1);
texture2D gMainTexture : register(t0);
RasterizerOrderedTexture2D<float> gTest : register(u0);

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

	//Light gLights[MaxLights];
};

cbuffer objectData : register(b1)
{
	float4x4 World;
	float4x4 TexTransform;
};

struct VertexIn
{
	float3 PosL : POSITION;
	float3 NormalL : NORMAL;
	float2 TexC : TEXCOORD;
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float2 TexC : TEXCOORD0;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout = (VertexOut)0.0f;
	vout.PosH = float4(vin.PosL,1.0f);
	//vout.PosH = mul(float4(vin.PosL, 1.0f), World);
	vout.PosH = mul(vout.PosH, gViewProj);
	vout.TexC = vin.TexC;

	//vout.TexC = mul(float4(vin.TexC, 0.0f, 1.0f), gInstanceData[MaterialIndex].MatTransform).xy;

	return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
	float4 litColor = float4(0,0,0,1);
	litColor = gMainTexture.Sample(gsamPointWrap, pin.TexC);

	litColor.a = 1.0f;
	return litColor;
}