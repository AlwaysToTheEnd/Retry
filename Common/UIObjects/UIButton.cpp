#include "UIButton.h"
#include "GraphicDO.h"
#include "PhysicsDO.h"

void UIButton::Init()
{
	m_Trans = CreateComponenet<DOTransform>();
	m_Font = CreateComponenet<DOFont>();
	m_UICollision = CreateComponenet<DOUICollision>();

	m_Font->SetFont(RenderFont::fontNames.front());
}

void UIButton::Update(float delta)
{
	if (m_isOnlyFontMode)
	{
		m_Font->m_Pos = m_Trans->GetTransform().p;

		m_Font->m_Pos.z -= 0.001f;
		m_Size = m_Font->m_DrawSize;
	}
	else
	{

	}

	SetUICollisionSize(m_UICollision);
}

void UIButton::SetPos(const physx::PxVec3& pos)
{
	m_Trans->SetPosX(pos.x - m_Size.x * m_BenchUV.x);
	m_Trans->SetPosY(pos.y - m_Size.y * m_BenchUV.y);
	m_Trans->SetPosZ(pos.z);
}

void UIButton::SetTexture(const std::string& name, const physx::PxVec2& size, bool colliderSizeIsEqualTexture)
{
	if (!m_Render)
	{
		m_Render = CreateComponenet<DORenderer>();
	}

	RenderInfo info(RENDER_TYPE::RENDER_UI);
	info.meshOrTextureName = name;
	info.uiInfo.size = size;
	info.uiInfo.uiType = UIBUTTON;
	m_Render->SetRenderInfo(info);

	m_Font->m_Text = L"";

	m_Size = size;

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

	m_Render = nullptr;
	m_isOnlyFontMode = true;
}

void UIButton::SetTextHeight(int height)
{
	m_Font->m_FontHeight = height;
}

void UIButton::SetSize(const physx::PxVec2& size)
{
	m_Size = size;

	RenderInfo& info = m_Render->GetRenderInfo();
	info.uiInfo.size = m_Size;

	if (m_UICollision)
	{
		SetUICollisionSize(m_UICollision);
	}
}

void UIButton::AddFunc(std::function<void()> func)
{
	m_UICollision->AddFunc(func);
}
