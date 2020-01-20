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
	return VertexBaseWork(normalVertex, World, NormalMapIndex);
}
#else

VertexOut VS(VertexIn vin)
{
    return VertexBaseWork(vin, World, NormalMapIndex);
}

#endif

float4 PS(VertexOut pin) : SV_Target
{
    float4 albedo = float4(0, 0, 0, 1);
	
    if (TextureIndex > -1)
    {
        albedo = GetTexel(TextureIndex, pin.TexC);
    }
    else if (MaterialIndex > -1)
    {
        albedo = gMaterialData[MaterialIndex].DriffuseAlbedo;
    }
	
    if (NormalMapIndex > -1)
    {
        float3 worldNormal = gMainTexture[NormalMapIndex].Sample(gsamLinearWrap, pin.TexC).rgb;
        worldNormal = normalize(mul(worldNormal, (float3x3) World));
        pin.Diffuse = dot(-gDirLight, worldNormal);
        pin.Reflection = reflect(gDirLight, worldNormal);
    }
	
    float3 diffuse =  albedo.rgb *saturate(pin.Diffuse);
    float3 viewDir = normalize(pin.ViewDir);
    float3 reflection = normalize(pin.Reflection);
    float3 specular = 0;
	
    if (diffuse.x > 0)
    {
        specular = saturate(dot(reflection, viewDir));
        specular = pow(specular, 10.0f);
    }
	
    float3 ambient = gAmbientLight.rgb * albedo.rgb;
	
    return float4(ambient + diffuse + specular, 1);
}