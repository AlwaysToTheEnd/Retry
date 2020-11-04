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
    int         ObjectID;
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
	float weights[8] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
	weights[0] = vin.BoneWeights1.x;
	weights[1] = vin.BoneWeights1.y;
	weights[2] = vin.BoneWeights1.z;
	weights[3] = vin.BoneWeights1.w;

	weights[4] = vin.BoneWeights2.x;
	weights[5] = vin.BoneWeights2.y;
	weights[6] = vin.BoneWeights2.z;
	weights[7] = vin.BoneWeights2.w;

	float3 posL = float3(0.0f, 0.0f, 0.0f);
	float3 normalL = float3(0.0f, 0.0f, 0.0f);

	for (int i = 0; i < 4; ++i)
	{
		posL += weights[i] * mul(float4(vin.PosL, 1.0f), gAniBoneMat[vin.BoneIndices1[i]]).xyz;
		normalL += weights[i] * mul(vin.NormalL, (float3x3)gAniBoneMat[vin.BoneIndices1[i]]);
	}

	for (int j = 0; j < 4; ++j)
	{
		posL += weights[j+4] * mul(float4(vin.PosL, 1.0f), gAniBoneMat[vin.BoneIndices2[j]]).xyz;
		normalL += weights[j+4] * mul(vin.NormalL, (float3x3)gAniBoneMat[vin.BoneIndices2[j]]);
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
    float4 diffuse = float4(1, 1, 1, 1);
    float3 normal = float3(0, 1, 0);
    float specPower = 1;
	
    if (TextureIndex > -1)
    {
        diffuse = GetTexel(TextureIndex, pin.TexC);
    }
    
    if (MaterialIndex > -1)
    {
        diffuse *= gMaterialData[MaterialIndex].DiffuseAlbedo;
        specPower = gMaterialData[MaterialIndex].SpecularExponent;
    }
	
    if (NormalMapIndex > -1)
    {
        normal = gMainTexture[NormalMapIndex].Sample(gsamLinearWrap, pin.TexC).rgb;
        normal = mul(normal, (float3x3) World);
    }
	else
    {
        normal = pin.Normal;
    }
	
    return GetPSOut(diffuse, normalize(normal), specPower, ObjectID);
}