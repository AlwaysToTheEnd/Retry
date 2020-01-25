#include "CullingFunc.hlsli"

struct ObjectData
{
    float4x4 World;
    float3 Scale;
    int TextureIndex;
    int NormalMapIndex;
    int MaterialIndex;
    int PrevAniBone;
    float BlendFactor;
    float BoundSphereRad;
    int pad0;
    int pad1;
    int pad2;
    float4 padding[9];
};

#ifdef SKINNED
struct IndirectCommand
{
    uint2   cbvAddress;
    uint2   aniboneAddress;
    uint4   drawArguments;
    uint    drawArguments2;
    uint3   pad;
};

#else
struct IndirectCommand
{
    uint2 cbvAddress;
    uint4 drawArguments;
    uint drawArguments2;
    uint pad;
};

#endif

StructuredBuffer<ObjectData> cbv : register(t0);
StructuredBuffer<IndirectCommand> inputCommands : register(t1);
AppendStructuredBuffer<IndirectCommand> outputCommands : register(u0);


bool CheckCulling(uint index)
{
    bool result = false;
    ObjectData data = cbv[index];
    float4 posW = float4(data.World._41_42_43, 1);
    
    if (type == 0)
    {
        float3 posV = mul(posW, viewMat).xyz;
        result = CullingByFrustum(posV, data.BoundSphereRad);
    }
    else if (type == 1)
    {
        result = CullingBySphere(posW.xyz, data.BoundSphereRad);
    }
    else if (type == 2)
    {
        result = CullingByBox(posW.xyz, data.BoundSphereRad);
    }
    else if (type == 3)
    {
        result = CullingByCon(posW.xyz, data.BoundSphereRad);
    }
    
    return result;
}

[numthreads(1, 1, 1)]
void CS(uint3 id : SV_DispatchThreadID)
{
    if (id.x < numObjects)
    {
        if (CheckCulling(id.x))
        {
            outputCommands.Append(inputCommands[id.x]);
        }
    }
}
