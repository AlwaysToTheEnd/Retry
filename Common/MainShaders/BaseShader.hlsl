#include "Header.hlsli"

struct MaterialData
{
	float4		DriffuseAlbedo;
	float3		FresnelR0;
	float		Roughness;
};

StructuredBuffer<MaterialData> gInstanceData : register(t0, space1);

cbuffer cbSkinned : register(b2)
{
	float4x4	gAniBoneMat[BONEMAXMATRIX];
};

cbuffer objectData : register(b1)
{
	float4x4	World;
	float3		Scale;
	int			TextureIndex;
    int			NormalMapIndex;
	uint		MaterialIndex;
	int			AniBoneIndex;
	int			PrevAniBone;
	float		blendFactor;
    int			pad0;
    int			pad1;
    int			pad2;
};

struct SkinnedVertex
{
	float3	PosL : POSITION;
	float3	NormalL : NORMAL;
	float2	TexC : TEXCOORD;
	float3	BoneWeights : WEIGHTS;
	uint4	BoneIndices : BONEINDICES;
};

struct VertexIn
{
	float3	PosL : POSITION;
	float3	NormalL : NORMAL;
	float2	TexC : TEXCOORD;
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float2 TexC : TEXCOORD0;
	float3 Normal : NORMAL0;
};

#ifdef SKINNED_VERTEX_SAHDER
VertexOut VS(SkinnedVertex vin)
{
	VertexOut vout = (VertexOut)0.0f;

	vout.TexC = vin.TexC;

	float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	weights[0] = vin.BoneWeights.x;
	weights[1] = vin.BoneWeights.y;
	weights[2] = vin.BoneWeights.z;
	weights[3] = 1.0f - weights[0] - weights[1] - weights[2];

	if (AniBoneIndex != -1)
	{
		float3 posL = float3(0.0f, 0.0f, 0.0f);
		//float3 normalL = float3(0.0f, 0.0f, 0.0f);
		//float3 tangentL = float3(0.0f, 0.0f, 0.0f);
		for (int i = 0; i < 4; ++i)
		{
			// Assume no nonuniform scaling when transforming normals, so 
			// that we do not have to use the inverse-transpose.

			posL += weights[i] * mul(float4(vin.PosL, 1.0f), gAniBoneMat[vin.BoneIndices[i]]).xyz;
			//normalL += weights[i] * mul(vin.NormalL, (float3x3)gAniBoneMat[AniBoneIndex].AniBoneMats[vin.BoneIndices[i]]);
			//tangentL += weights[i] * mul(vin.TangentL.xyz, (float3x3)gAniBoneMat[AniBoneIndex].AniBoneMats[vin.BoneIndices[i]]);
		}

		vin.PosL = posL;
	}

	vout.PosH = mul(float4(vin.PosL, 1.0f), World);
	vout.PosH = mul(vout.PosH, gViewProj);

	vout.Normal=  mul(float4(vin.NormalL, 1.0f), World).xyz;
	return vout;
}
#else

VertexOut VS(VertexIn vin)
{
	VertexOut vout = (VertexOut)0.0f;

	vout.TexC = vin.TexC;

    vout.PosH = float4((vin.PosL * Scale), 1.0f);
    vout.PosH = mul(vout.PosH, World);
	vout.PosH = mul(vout.PosH, gViewProj);

    vout.Normal = mul(float4(vin.NormalL, 1.0f), World);
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
	
    litColor.rgb *= dot(normalize(pin.Normal), -normalize(float3(-1, -1, -1)));
	return litColor;
}