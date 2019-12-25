#include "UIPanel.h"
#include "BaseComponent.h"
#include "GraphicComponent.h"
#include "CGHScene.h"

void UIPanel::Init()
{
	m_Trans = AddComponent<ComTransform>();
	m_Font = AddComponent<ComFont>();
	m_UICollision = AddComponent<ComUICollision>();
	m_Render = AddComponent<ComRenderer>();

	m_Font->SetFont(RenderFont::fontNames.front());
	m_Font->m_FontHeight = 10;
	RenderInfo info(RENDER_UI);
	info.point.size = { 100,100,0 };
	m_Render->SetRenderInfo(info);

	m_UIOffButton = new UIButton(GetScene());
	GetScene().AddGameObjects(m_UIOffButton);
	//#TODO need to add texture of UIcloseButton;
	m_UIOffButton->SetTexture("ice.dds", { 5.0f, 5.0f });
	m_UIOffButton->AddFunc(std::bind(&UIPanel::UIOff, this));
}


void UIPanel::AddUICom(unsigned int x, unsigned y, UIButton* button)
{
	m_UIComs.push_back({ UICOMTYPE::UIBUTTON, button });
	m_UIComOffset.push_back({ static_cast<float>(x),static_cast<float>(y) });
}

void UIPanel::AddUICom(unsigned int x, unsigned y, UIParam* param)
{
	m_UIComs.push_back({ UICOMTYPE::UIPARAM, param });
	m_UIComOffset.push_back({ static_cast<float>(x),static_cast<float>(y) });
}

void UIPanel::SetBackGroundTexture(const std::string& name)
{
	RenderInfo info = m_Render->GetRenderInfo();
	info.meshOrTextureName = name;
	m_Render->SetRenderInfo(info);
}

void UIPanel::SetBackGroundColor(DirectX::XMFLOAT4 color)
{
	RenderInfo info = m_Render->GetRenderInfo();
	info.meshOrTextureName = "";
	info.point.color = color;
	m_Render->SetRenderInfo(info);
}

void UIPanel::SetSize(unsigned int x, unsigned y)
{
	m_Size = physx::PxVec2(x, y);
	auto halfSize = m_Size / 2;

	RenderInfo info = m_Render->GetRenderInfo();
	info.point.size = { halfSize.x, halfSize.y,0 };
	m_UICollision->SetSize({ halfSize.x, halfSize.y });
	m_Render->SetRenderInfo(info);
}

void UIPanel::SetName(const std::wstring& name)
{
	m_Font->m_Text = name;
}

void UIPanel::SetPos(DirectX::XMFLOAT3 pos)
{
	physx::PxTransform wPos = m_Trans->GetTransform();
	wPos.p = { pos.x, pos.y, pos.z };

	m_Trans->SetTransform(wPos);
}

void UIPanel::UIOn()
{
	m_Active = true;

	SetAllComponentActive(true);
	m_UIOffButton->SetAllComponentActive(true);

	for (auto& it : m_UIComs)
	{
		it.object->SetAllComponentActive(true);
	}
}

void UIPanel::UIOff()
{
	m_Active = false;

	SetAllComponentActive(false);
	m_UIOffButton->SetAllComponentActive(false);

	for (auto& it : m_UIComs)
	{
		it.object->SetAllComponentActive(false);
	}
}

void UIPanel::Update()
{
	if (m_Active)
	{
		physx::PxTransform panelTransform = m_Trans->GetTransform();
		auto halfSize = m_Size / 2;
		m_UIOffButton->GetComponent<ComTransform>()->SetTransform(
			physx::PxTransform(halfSize.x - 5, -halfSize.y + 5, -0.01f) * panelTransform);
		m_Font->m_OffsetPos.x = -halfSize.x;
		m_Font->m_OffsetPos.y = -halfSize.y;

		for (size_t i = 0; i < m_UIComs.size(); i++)
		{
			auto transform = m_UIComs[i].object->GetComponent<ComTransform>();

			transform->SetTransform(physx::PxTransform(
				m_UIComOffset[i].x - halfSize.x, m_UIComOffset[i].y - halfSize.y, -0.01f) *
				panelTransform);
		}
	}
}
