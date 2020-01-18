#include "Header.hlsli"

cbuffer objectData : register(b1)
{
    float4x4    World;
    float3      Scale;
    int         TextureIndex;
    int         NormalMapIndex;
    int         MaterialIndex;
    uint        MapSize;
    uint        NumVertices;
};

StructuredBuffer<float> gHeights : register(t1, space1);

struct HeightFieldGSIn
{
    float   height  : GSHEIGHT;
    uint id : VERTEXINDEX;
};

HeightFieldGSIn VS(float vin : HEIGHT, uint id:SV_VertexID)
{
    HeightFieldGSIn vOut;
    vOut.height = vin;
    vOut.id = id;
    return vOut;
}

[maxvertexcount(4)]
void GS(point HeightFieldGSIn input[1], inout TriangleStream<VertexOut> output)
{
    uint id = input[0].id;
    uint indexX = id % MapSize;
    uint indexY = id / MapSize;
    
    if (indexX < (MapSize - 1) && indexY < (MapSize - 1))
    {
        VertexOut vertices[4];
        vertices[0].TexC.x = float(indexX) / (MapSize - 1);
        vertices[0].TexC.y = float(indexY) / (MapSize - 1);
        vertices[0].PosH.y = input[0].height;
        vertices[0].PosH.z = indexY;
        vertices[0].PosH.x = indexX;
        vertices[0].PosH.w = 1;
           
        vertices[1].TexC.x = vertices[0].TexC.x;
        vertices[1].TexC.y = float(indexY + 1) / (MapSize - 1);
        vertices[1].PosH.y = gHeights[id + MapSize];
        vertices[1].PosH.z = indexY+1;
        vertices[1].PosH.x = indexX;
        vertices[1].PosH.w = 1;
       
        vertices[2].TexC.x = float(indexX+1) / (MapSize - 1);
        vertices[2].TexC.y = vertices[0].TexC.y;
        vertices[2].PosH.y = gHeights[id + 1];
        vertices[2].PosH.z = indexY;
        vertices[2].PosH.x = indexX + 1;
        vertices[2].PosH.w = 1;
        
        vertices[3].TexC.x = vertices[2].TexC.x;
        vertices[3].TexC.y = vertices[1].TexC.y;
        vertices[3].PosH.y = gHeights[id + MapSize + 1];
        vertices[3].PosH.z = indexY + 1;
        vertices[3].PosH.x = indexX + 1;
        vertices[3].PosH.w = 1;
        
        [unroll]
        for (int i = 0; i < 4; i++)
        {
            //vertices[i].PosH.y = 1;
            vertices[i].PosH.xyz *= Scale;
            vertices[i].PosH = mul(vertices[i].PosH, World);
            vertices[i].PosH = mul(vertices[i].PosH, gViewProj);
        }
        
        output.Append(vertices[0]);
        output.Append(vertices[1]);
        output.Append(vertices[2]);
        output.Append(vertices[3]);
    }
}

float4 PS(VertexOut pin) : SV_Target
{
    float4 litColor = float4(0, 1, 0, 1);

    if (TextureIndex >= 0)
    {
        litColor = GetTexel(TextureIndex, pin.TexC);
    }

    clip(litColor.a);
	
    //litColor.rgb *= dot(normalize(pin.Normal), -normalize(float3(-1, -1, -1)));
    return litColor;
}