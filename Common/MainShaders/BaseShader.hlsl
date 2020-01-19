#include "Header.hlsli"

cbuffer objectData : register(b1)
{
	float4x4	World;
	float3		Scale;
	int			TextureIndex;
    int			NormalMapIndex;
    int			MaterialIndex;
	int			PrevAniBone;
	float		blendFactor;
};

#ifdef SKINNED_VERTEX_SAHDER
cbuffer cbSkinned : register(b2)
{
	float4x4	gAniBoneMat[BONEMAXMATRIX];
};

VertexOut VS(SkinnedVertexIn vin)
{
	VertexOut vout = (VertexOut)0.0f;

	vout.TexC = vin.TexC;

	float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	weights[0] = vin.BoneWeights.x;
	weights[1] = vin.BoneWeights.y;
	weights[2] = vin.BoneWeights.z;
	weights[3] = 1.0f - weights[0] - weights[1] - weights[2];

	float3 posL = float3(0.0f, 0.0f, 0.0f);
	float3 normalL = float3(0.0f, 0.0f, 0.0f);

	for (int i = 0; i < 4; ++i)
	{
		posL += weights[i] * mul(float4(vin.PosL, 1.0f), gAniBoneMat[vin.BoneIndices[i]]).xyz;
		normalL += weights[i] * mul(vin.NormalL, (float3x3)gAniBoneMat[vin.BoneIndices[i]]);
	}

	vin.PosL = posL;

	vout.PosH = mul(float4(vin.PosL, 1.0f), World);
	vout.PosH = mul(vout.PosH, gViewProj);

	float3 worldNormal = normalize(mul(vin.NormalL, (float3x3) World));
    vout.Diffuse =  saturate(dot(-gDirLight, worldNormal));
	return vout;
}
#else

VertexOut VS(VertexIn vin)
{
	VertexOut vout = (VertexOut)0.0f;

	vout.TexC = vin.TexC;

    vout.PosH = float4(vin.PosL, 1.0f);
    vout.PosH = mul(vout.PosH, World);
	vout.PosH = mul(vout.PosH, gViewProj);

    float3 worldNormal = normalize(mul(vin.NormalL, (float3x3) World));
    vout.Diffuse = saturate(dot(-gDirLight, worldNormal));
	
	return vout;
}

#endif

float4 PS(VertexOut pin) : SV_Target
{
	float4 litColor = float4(0,1,0,1);

    if (TextureIndex >= 0)
    {
        litColor = GetTexel(TextureIndex, pin.TexC);
    }

	clip(litColor.a);
	
    litColor.rgb *= (gAmbientLight.rgb + pin.Diffuse).rgb;
	return litColor;
}