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
    float3 BoneWeights : WEIGHTS;
    uint4 BoneIndices : BONEINDICES;
};

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float2 TexC : TEXCOORD0;
    float Diffuse : TEXCOORD1;
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