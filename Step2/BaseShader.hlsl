struct MaterialData
{
	float4 DriffuseAlbedo;
	float3 FresnelR0;
	float Roughness;
	float4x4 MatTransform;
};

StructuredBuffer<MaterialData> gInstanceData: register(t0, space1);
texture2D gMainTexture : register(t0);

cbuffer cbPass : register(b0)
{
	float4x4 gViewProj;
};

cbuffer objectData : register(b1)
{
	float4x4 World;
	uint MaterialIndex;
	uint InstPad0;
	uint InstPad1;
	uint InstPad2;
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
	vout.PosH = mul(float4(vin.PosL, 1.0f), World);
	vout.PosH = mul(vout.PosH, gViewProj);

	vout.TexC = mul(float4(vin.TexC, 0.0f, 1.0f), gInstanceData[MaterialIndex].MatTransform).xy;

	return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
	float4 litColor = float4(0,0,0,0);

	//litColor = gMainTexture.Sample(pin.TexC);

	return litColor;
}