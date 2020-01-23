
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

#ifdef SKINNED

struct IndirectCommand
{
    uint2 cbvAddress;
    uint2 boneCBAddress;
    uint4 drawArguments;
};

#else
struct IndirectCommand
{
    uint2 cbvAddress;
    uint4 drawArguments;
};

#endif

int numObjects :                                            register(c0);

StructuredBuffer<objectData> cbv :                          register(t0);
StructuredBuffer<IndirectCommand> inputCommands :           register(t1);
AppendStructuredBuffer<IndirectCommand> outputCommands :    register(u0);

[numthreads(1, 1, 1)]
void CS(uint3 id : SV_DispatchThreadID)
{
    //#TODO Culling.
    
    if(id.x < numObjects)
    {
        outputCommands.Append(inputCommands[id.x]);
    }
}
