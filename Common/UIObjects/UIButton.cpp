#include "UIButton.h"
#include "GraphicComponent.h"
#include "BaseComponent.h"

void UIButton::Init()
{
	m_Trans = AddComponent<ComTransform>();
	m_Font = AddComponent<ComFont>();
	m_Render = AddComponent<ComRenderer>();
	m_UICollision = AddComponent<ComUICollision>();
	m_Font->SetFont(RenderFont::fontNames.front());
}

void UIButton::Update()
{
	physx::PxVec3 pos = m_Trans->GetTransform().p;
	//DirectX::XMFLOAT3 size = m_Render->GetRenderInfo().texPoint.size;
	m_Font->m_Pos.x = pos.x;
	m_Font->m_Pos.y = pos.y;
}

void UIButton::SetTexture(const std::string& name, const DirectX::XMFLOAT2& halfSize, bool colliderSizeIsEqualTexture)
{
	RenderInfo info(RENDER_TYPE::RENDER_UI);
	info.meshOrTextureName = name;
	info.texPoint.size = { halfSize.x, halfSize.y, 0.0f };
	m_Render->SetRenderInfo(info);

	if (colliderSizeIsEqualTexture)
	{
		m_UICollision->SetSize(halfSize);
	}
}

void UIButton::SetText(const std::wstring& text)
{
	m_Font->m_Text = text;
}

void UIButton::SetTextHeight(int height)
{
	m_Font->m_FontHeight = height;
}

void UIButton::SetColliderSize(const DirectX::XMFLOAT2& halfSize)
{
	m_UICollision->SetSize(halfSize);
}

void UIButton::AddFunc(std::function<void()> func)
{
	m_UICollision->AddFunc(func);
}
