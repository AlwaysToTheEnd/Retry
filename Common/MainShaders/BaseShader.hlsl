
struct MaterialData
{
	float4		DriffuseAlbedo;
	float3		FresnelR0;
	float		Roughness;
	int			DiffuseMapIndex;
	int			NormalMapIndex;
	uint		MaterialPad1;
	uint		MaterialPad2;
};

StructuredBuffer<MaterialData> gInstanceData : register(t0, space1);
StructuredBuffer<float4x4> gAniBoneMat : register(t1, space1);
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

	//Light gLights[MaxLights];
};

cbuffer objectData : register(b1)
{
	float4x4	World;
	uint		MaterialIndex;
	int			AniBoneIndex;
	uint		Pad0;
	uint		Pad1;
};

struct VertexIn
{
	float3	PosL : POSITION;
	float3	NormalL : NORMAL;
	float2	TexC : TEXCOORD;
	float3	BoneWeights : WEIGHTS;
	uint4	BoneIndices : BONEINDICES;
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float2 TexC : TEXCOORD0;
};

VertexOut VS(VertexIn vin)
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

	vout.PosH = mul(float4(vin.PosL, 1.0f), gViewProj);
	//vout.PosH = mul(float4(vin.PosL, 1.0f), World);
	//vout.PosH = mul(vout.PosH, gViewProj);

	return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
	float4 litColor = float4(0,0,0,1);
	litColor = gMainTexture[0].Sample(gsamPointWrap, pin.TexC);

	litColor.a = 1.0f;
	return litColor;
}