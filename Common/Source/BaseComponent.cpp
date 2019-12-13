#include "BaseComponent.h"

using namespace DirectX;
std::vector<CGH::MAT16>* ComTransform::m_TransformMats = nullptr;

ComPhysics::ComPhysics(GameObject& gameObject, int ID)
	:IComponent(COMPONENTTYPE::COM_PHYSICS, gameObject, ID)
{
}

void ComPhysics::Update()
{
}

void ComTransform::Update()
{
	XMVECTOR pos = XMLoadFloat3(&m_Pos);
	XMVECTOR scale = XMLoadFloat3(&m_Scale);
	XMVECTOR quter = XMLoadFloat4(&m_Quter);
	XMVECTOR zero = XMVectorZero();

	XMStoreFloat4x4(GetNoneConstMat(), XMMatrixAffineTransformation(scale, zero, quter, pos));
}

void ComTransform::SetMatrix(CGH::MAT16& mat)
{

}
