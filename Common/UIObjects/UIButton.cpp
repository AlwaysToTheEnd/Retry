#include "UIButton.h"
#include "GraphicComponent.h"
#include "BaseComponent.h"

void UIButton::Init()
{
	m_Trans = AddComponent<ComTransform>();
	m_Font = AddComponent<ComFont>();
	m_UICollision = AddComponent<ComUICollision>();

	m_Font->SetFont(RenderFont::fontNames.front());
	m_Font->SetBenchmark(RenderFont::FONTBENCHMARK::CENTER);
}

void UIButton::Update(unsigned long long delta)
{
	m_Font->m_Pos = m_Trans->GetTransform().p;

	m_Font->m_Pos.z -= 0.001f;

	if (m_isOnlyFontMode)
	{
		m_UICollision->SetSize({ m_Font->m_DrawSize.x / 2, m_Font->m_DrawSize.y / 2 });
	}
}

void UIButton::SetTexture(const std::string& name, const physx::PxVec2& halfSize, bool colliderSizeIsEqualTexture)
{
	if (!m_Render)
	{
		m_Render = AddComponent<ComRenderer>();
	}

	RenderInfo info(RENDER_TYPE::RENDER_UI);
	info.meshOrTextureName = name;
	info.texPoint.size = { halfSize.x, halfSize.y, 0.0f };
	m_Render->SetRenderInfo(info);

	if (colliderSizeIsEqualTexture)
	{
		m_UICollision->SetSize(halfSize);
	}

	m_isOnlyFontMode = false;
}

void UIButton::SetText(const std::wstring& text)
{
	m_Font->m_Text = text;
}

void UIButton::OnlyFontMode()
{
	if (m_Render)
	{
		DeleteComponent(m_Render);
	}

	m_Render = nullptr;
	m_isOnlyFontMode = true;
}

void UIButton::SetTextHeight(int height)
{
	m_Font->m_FontHeight = height;
}

void UIButton::SetColliderSize(const physx::PxVec2& halfSize)
{
	m_UICollision->SetSize(halfSize);
}

void UIButton::AddFunc(std::function<void()> func)
{
	m_UICollision->AddFunc(func);
}
