
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
    uint2   cbvAddress;
    uint4   drawArguments;
    uint    drawArguments2;
    uint    pad;
};

#endif

/*
enum DX12_COMPUTE_CULLING_TYPE
{
	DX12_COMPUTE_CULLING_TYPE_FRUSTUM,
	DX12_COMPUTE_CULLING_TYPE_SPHERE,
	DX12_COMPUTE_CULLING_TYPE_BOX,
	DX12_COMPUTE_CULLING_TYPE_CON,
};

struct DX12_COMPUTE_CULLING_FRUSTUM
{
	physx::PxVec4	rightNormal;
	physx::PxVec4	leftNormal;
	physx::PxVec4	upNormal;
	physx::PxVec4	downNormal;
	physx::PxMat44	viewMat;
	physx::PxVec2	near_Far;
};

struct DX12_COMPUTE_CULLING_SPHERE
{
    physx::PxVec4 pos_Radian;
};

struct DX12_COMPUTE_CULLING_BOX
{
    physx::PxVec4 pos;
    physx::PxVec4 halfSize;
};

struct DX12_COMPUTE_CULLING_CON
{
    physx::PxVec4 pos_Length;
    physx::PxVec4 dir_cos;
};*/

cbuffer CullingDesc :                                       register(b0)
{
    float4      rightNormal;
    float4      leftNormal;
    float4      upNormal;
    float4      downNormal;
    float4x4    viewMat;
    float2      near_Far;
    uint        type;
    uint        numObjects;
};

StructuredBuffer<ObjectData> cbv : register(t0);

StructuredBuffer<IndirectCommand> inputCommands :           register(t1);
AppendStructuredBuffer<IndirectCommand> outputCommands :    register(u0);

bool CheckInPlane(float3 normal, float3 pos, float rad)
{
    return dot(normal, pos) <= rad;
}

void CullingByFrustum(uint index)
{
    ObjectData data = cbv[index];
    float4 pos = float4(data.World._41_42_43, 1);
    pos = mul(pos, viewMat);
    if (pos.z + data.BoundSphereRad > near_Far.x && pos.z - data.BoundSphereRad < near_Far.y)
    {
        if (CheckInPlane(rightNormal.xyz, pos.xyz, data.BoundSphereRad) &&
            CheckInPlane(leftNormal.xyz, pos.xyz, data.BoundSphereRad) &&
            CheckInPlane(upNormal.xyz, pos.xyz, data.BoundSphereRad) &&
            CheckInPlane(downNormal.xyz, pos.xyz, data.BoundSphereRad))
        {
            outputCommands.Append(inputCommands[index]);
        }
    }
}

void CullingBySphere(uint index)
{
    ObjectData data = cbv[index];
    float4 pos = float4(data.World._41_42_43, 1);
    
    float3 cullSpherePos = rightNormal.xyz;
    float cullSphereRad = rightNormal.w;
    
    float3 dir = pos.xyz - cullSpherePos;
    dir *= dir;
    float squaredDistance = dir.x + dir.y + dir.z - (data.BoundSphereRad * data.BoundSphereRad);

    if (squaredDistance <= (cullSphereRad * cullSphereRad))
    {
        outputCommands.Append(inputCommands[index]);
    }
}

void CullingByBox(uint index)
{
    ObjectData data = cbv[index];
    float3 pos = data.World._41_42_43;
    float3 boxCenter = rightNormal.xyz;
    float3 boxHalfSize = leftNormal.xyz;
    
    if (pos.x + data.BoundSphereRad > (boxCenter.x - boxHalfSize.x) &&
        pos.x - data.BoundSphereRad < (boxCenter.x + boxHalfSize.x) &&
        pos.y + data.BoundSphereRad > (boxCenter.y - boxHalfSize.y) &&
        pos.y - data.BoundSphereRad < (boxCenter.y + boxHalfSize.y) &&
        pos.z + data.BoundSphereRad > (boxCenter.z - boxHalfSize.z) &&
        pos.z - data.BoundSphereRad < (boxCenter.z + boxHalfSize.z))
    {
        outputCommands.Append(inputCommands[index]);
    }
}

void CullingByCon(uint index)
{
    ObjectData data = cbv[index];
    float3 pos = data.World._41_42_43;

    float3 conOrigin = rightNormal.xyz;
    float3 conDir = normalize(leftNormal.xyz);
    float conRength = rightNormal.w;
    float angle = leftNormal.w;
    
    float originToObjectDir = pos - conOrigin;
    
    float dotValue = dot(conDir, originToObjectDir);
    
    if(dotValue>0 && dotValue< conRength)
    {
        float3 verticalToCondirPos = conOrigin + (conDir * dotValue);
        float3 objectBundPos = pos + (normalize(verticalToCondirPos - pos) * data.BoundSphereRad);

        float toObjectAngle = dot(conDir, normalize(objectBundPos - conOrigin));
        
        if (angle >= abs(toObjectAngle))
        {
            outputCommands.Append(inputCommands[index]);
        }
    }
}

[numthreads(1, 1, 1)]
void CS(uint3 id : SV_DispatchThreadID)
{
    if (id.x < numObjects)
    {
        if (type == 0)
        {
            CullingByFrustum(id.x);
        }
        else if (type == 1)
        {
            CullingBySphere(id.x);
        }
        else if (type == 2)
        {
            CullingByBox(id.x);
        }
        else if (type == 3)
        {
            CullingByCon(id.x);
        }
    }
}
