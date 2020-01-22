
struct objectData
{
    float4x4 World;
    float3 Scale;
    int TextureIndex;
    int NormalMapIndex;
    int MaterialIndex;
    int PrevAniBone;
    float BlendFactor;
    float4 padding[10];
};

struct IndirectCommand
{
    uint2 cbvAddress;
    uint4 drawArguments;
};

StructuredBuffer<objectData> cbv : register(t0);
StructuredBuffer<IndirectCommand> inputCommands : register(t1);
AppendStructuredBuffer<IndirectCommand> outputCommands : register(u0);

[numthreads(16, 1, 1)]
void CS(uint3 groupId : SV_GroupID, uint groupIndex : SV_GroupIndex)
{

}
