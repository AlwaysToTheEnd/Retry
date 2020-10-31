struct VertexIn
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
    float2 TexC : TEXCOORD;
};

struct SkinnedVertexIn
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
    float2 TexC : TEXCOORD;
    float4 BoneWeights1 : WEIGHTS0;
    float4 BoneWeights2 : WEIGHTS1;
    uint4 BoneIndices1 : BONEINDICES0;
    uint4 BoneIndices2 : BONEINDICES1;
};

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float2 TexC : TEXCOORD0;
    float3 Normal : TEXCOORD2;
};

struct POINTVertexIn
{
    uint type : MESHTYPE;
    uint cbIndex : CBINDEX;
    float3 size : MESHSIZE;
    float4 color : MESHCOLOR;
};

struct POINTVertexOut
{
    float4 posH : SV_POSITION;
    float4 color : COLOR0;
    nointerpolation int texIndex : MATNDEX;
};

struct UIVertexIn
{
    float4 color : UICOLOR;
    float3 pos : UIPOS;
    float2 size : UISIZE;
    int uiType : UITYPE;
    int textureIndex : UITEXTURE;
};

struct PSOut
{
    float4 color : SV_Target;
    float4 normal : SV_Target1;
    float4 specPow : SV_Target2;
};