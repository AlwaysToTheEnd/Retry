#include "InOutStructs.hlsli"

StructuredBuffer<float> InputHeights : register(t0);
RWStructuredBuffer<VertexIn> Vertices : register(u0);
RWStructuredBuffer<uint> Indices : register(u1);

cbuffer FieldAttribute : register(b0)
{
    float3 Scale;
    uint MapSize;
    uint NumVertices;
}

[numthreads(1, 1, 1)]
void SettingVerticesPos(uint3 id : SV_DispatchThreadID)
{
    uint i = id.x;

    uint indexX = i % MapSize;
    uint indexY = i / MapSize;
    
    Vertices[i].TexC.x = float(indexX) / (MapSize - 1);
    Vertices[i].TexC.y = float(indexY) / (MapSize - 1);

    Vertices[i].PosL.y = InputHeights[i];
    Vertices[i].PosL.z = MapSize - indexY - 1;
    Vertices[i].PosL.x = indexX;
        
    Vertices[i].PosL *= Scale;
}

[numthreads(1, 1, 1)]
void SettingVerticesNormalAndIndices(uint3 id : SV_DispatchThreadID)
{
    uint i = id.x;
 
    uint indexX = i % MapSize;
    uint indexY = i / MapSize;
    
    if (indexX < (MapSize - 1) && indexY < (MapSize - 1))
    {
            
        float3 rightVec = Vertices[i + 1].PosL - Vertices[i].PosL;
        float3 upVec = Vertices[i + MapSize].PosL - Vertices[i].PosL;

        float3 normalVec = cross(rightVec, upVec);

        Vertices[i].NormalL = normalize(normalVec);
            
        uint indexBase = i * 6;
        Indices[indexBase + 0] = i;
        Indices[indexBase + 1] = i + 1;
        Indices[indexBase + 2] = i + MapSize;
    
        Indices[indexBase + 3] = i + MapSize;
        Indices[indexBase + 4] = i + 1;
        Indices[indexBase + 5] = i + 1 + MapSize;
    }
    else
    {
        Vertices[i].NormalL = float3(0, 1, 0);
    }
}