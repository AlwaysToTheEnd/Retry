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
		m_UICollision->SetOffset({ m_Font->m_DrawSize.x / 2, 0 });
		m_Size = m_Font->m_DrawSize;
	}
}

void UIButton::SetPos(const physx::PxVec3& pos)
{
	physx::PxVec2 uv = m_BenchUV - physx::PxVec2(0.5f, 0.5f);
	m_Trans->SetPosX(pos.x - m_Size.x * uv.x);
	m_Trans->SetPosY(pos.y - m_Size.y * uv.y);
	m_Trans->SetPosZ(pos.z);
}

void UIButton::SetTexture(const std::string& name, const physx::PxVec2& halfSize, bool colliderSizeIsEqualTexture)
{
	if (!m_Render)
	{
		m_Render = CreateComponenet<DORenderer>();
	}

	RenderInfo info(RENDER_TYPE::RENDER_UI);
	info.meshOrTextureName = name;
	info.uiInfo.size = halfSize;
	info.uiInfo.uiType = UIBUTTON;
	m_Render->SetRenderInfo(info);

	if (colliderSizeIsEqualTexture)
	{
		m_UICollision->SetSize(halfSize);
	}

	m_Size.x = halfSize.x * 2;
	m_Size.y = halfSize.y * 2;

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
		m_Render->Delete();
	}

	m_Font->SetBenchmark(RenderFont::FONTBENCHMARK::LEFT);
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
