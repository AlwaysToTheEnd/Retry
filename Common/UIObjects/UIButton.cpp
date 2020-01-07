#include "UIButton.h"
#include "GraphicDO.h"
#include "PhysicsDO.h"

void UIButton::Init()
{
	m_Trans = CreateComponenet<DOTransform>();
	m_Font = CreateComponenet<DOFont>();
	m_UICollision = CreateComponenet<DOUICollision>();

	m_Font->SetFont(RenderFont::fontNames.front());
	m_Font->SetBenchmark(RenderFont::FONTBENCHMARK::CENTER);
}

void UIButton::Update(float delta)
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
		m_Render = CreateComponenet<DORenderer>();
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
		ExceptComponent(m_Render);
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
