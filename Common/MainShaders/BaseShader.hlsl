#include "Header.hlsli"

cbuffer objectData : register(b1)
{
	float4x4	World;
	float3		Scale;
	int			TextureIndex;
    int			NormalMapIndex;
    int			MaterialIndex;
	int			PrevAniBone;
	float		BlendFactor;
    float       BoundSphereRad;
    int         pad0;
    int         pad1;
    int         pad2;
};

#ifdef SKINNED_VERTEX_SAHDER
cbuffer cbSkinned : register(b2)
{
	float4x4	gAniBoneMat[BONEMAXMATRIX];
};

VertexOut VS(SkinnedVertexIn vin)
{
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
	
	VertexIn normalVertex;
	normalVertex.TexC = vin.TexC;
	normalVertex.PosL = posL;
	normalVertex.NormalL = normalL;

	return VertexBaseWork(normalVertex, World);
}
#else

VertexOut VS(VertexIn vin)
{
    return VertexBaseWork(vin, World);
}

#endif

PSOut PS(VertexOut pin)
{
    float4 diffuse = float4(0, 0, 0, 1);
    float3 normal = float3(0, 1, 0);
	
    if (TextureIndex > -1)
    {
        diffuse = GetTexel(TextureIndex, pin.TexC);
    }
    else if (MaterialIndex > -1)
    {
        diffuse = gMaterialData[MaterialIndex].DriffuseAlbedo;
    }
	
    if (NormalMapIndex > -1)
    {
        normal = gMainTexture[NormalMapIndex].Sample(gsamLinearWrap, pin.TexC).rgb;
        normal = normalize(mul(normal, (float3x3) World));
    }
	else
    {
        normal = normalize(pin.Normal);
    }
	
    return GetPSOut(diffuse, normal, 1);
}