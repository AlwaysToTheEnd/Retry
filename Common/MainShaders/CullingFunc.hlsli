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

cbuffer CullingDesc : register(b0)
{
    float4 rightNormal;
    float4 leftNormal;
    float4 upNormal;
    float4 downNormal;
    float4x4 viewMat;
    float2 near_Far;
    uint type;
    uint numObjects;
};

bool CheckInPlane(float3 normal, float3 pos, float rad)
{
    return dot(normal, pos) <= rad;
}

bool CullingByFrustum(float3 posV, float rad)
{
    bool result = false;
    
    if (posV.z + rad > near_Far.x && posV.z - rad < near_Far.y)
    {
        if (CheckInPlane(rightNormal.xyz, posV, rad) &&
            CheckInPlane(leftNormal.xyz, posV, rad) &&
            CheckInPlane(upNormal.xyz, posV, rad) &&
            CheckInPlane(downNormal.xyz, posV, rad))
        {
            result = true;
        }
    }
    
    return result;
}

bool CullingBySphere(float3 posW, float rad)
{
    float3 cullSpherePos = rightNormal.xyz;
    float cullSphereRad = rightNormal.w;
    
    float3 dir = posW - cullSpherePos;
    dir *= dir;
    float squaredDistance = dir.x + dir.y + dir.z - (rad * rad);
    int test = 1;
 
    return squaredDistance <= (cullSphereRad * cullSphereRad);
}

bool CullingByBox(float3 posW, float rad)
{
    bool result = false;
    float3 boxCenter = rightNormal.xyz;
    float3 boxHalfSize = leftNormal.xyz;
    
    if (posW.x + rad > (boxCenter.x - boxHalfSize.x) &&
        posW.x - rad < (boxCenter.x + boxHalfSize.x) &&
        posW.y + rad > (boxCenter.y - boxHalfSize.y) &&
        posW.y - rad < (boxCenter.y + boxHalfSize.y) &&
        posW.z + rad > (boxCenter.z - boxHalfSize.z) &&
        posW.z - rad < (boxCenter.z + boxHalfSize.z))
    {
        result = true;
    }
    
    return result;
}

bool CullingByCon(float3 posW, float rad)
{
    bool result = false;

    float3 conOrigin = rightNormal.xyz;
    float3 conDir = normalize(leftNormal.xyz);
    float conRength = rightNormal.w;
    float angle = leftNormal.w;
    
    float3 originToObjectDir = posW - conOrigin;
    
    float dotValue = dot(conDir, originToObjectDir);
    
    if (dotValue > 0 && dotValue < conRength)
    {
        float3 verticalToCondirPos = conOrigin + (conDir * dotValue);
        float3 objectBundPos = posW + (normalize(verticalToCondirPos - posW) * rad);

        float toObjectAngle = dot(conDir, normalize(objectBundPos - conOrigin));
        
        if (angle >= abs(toObjectAngle))
        {
            result = true;
        }
    }
    
    return result;
}