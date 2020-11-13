#include "UIPanel.h"
#include "PhysicsDO.h"
#include "GraphicDO.h"
#include "CGHScene.h"
#include "d3dApp.h"

UIPanel::UIPanelController UIPanel::s_PanelController;

void UIPanel::Init()
{
	m_Trans = CreateComponenet<DOTransform>();
	m_Font = CreateComponenet<DOFont>();
	m_Render = CreateComponenet<DORenderer>();

	m_Trans->SetPosZ(0.8f);
	m_Font->SetFont(RenderFont::fontNames.front());
	m_Font->m_FontHeight = CGH::GO.ui.panelTitleTextHeight;
	auto& render = m_Render->GetRenderInfo();
	render.type = RENDER_UI;
	render.uiInfo.uiType = UIPANEL;
	render.uiInfo.color = { 0.2f, 0.2f, 0.2f, 0.7f };

	m_Render->AddPixelFunc(std::bind(&UIPanelController::SortPanels, &s_PanelController, this),
		DirectX::Mouse::ButtonStateTracker::PRESSED, DirectX::MOUSEBUTTONINDEX::LEFTBUTTON);
}

void UIPanel::Delete()
{
	s_PanelController.DeletedPanel(this);
	DeleteAllComs();

	GameObject::Delete();
}

void UIPanel::AddUICom(UIObject* ui)
{
	bool isHave = false;

	for (auto& it : m_UIComs)
	{
		if (it == ui)
		{
			isHave = true;
			break;
		}
	}

	if (!isHave)
	{
		m_UIComs.push_back(ui);
		ui->SetParent(this);
	}
}

void UIPanel::AddPixelFunc(std::function<void()> func, DirectX::Mouse::ButtonStateTracker::ButtonState state, DirectX::MOUSEBUTTONINDEX index)
{
	m_Render->AddPixelFunc(func, state, index);
}

void UIPanel::DeleteAllComs()
{
	auto UIComs = m_UIComs;

	for (auto& it : UIComs)
	{
		it->Delete();
	}

	m_UIComs.clear();
}

void UIPanel::PopUICom(const UIObject* uiCom)
{
	for (size_t i = 0; i < m_UIComs.size(); i++)
	{
		if (m_UIComs[i] == uiCom)
		{
			m_UIComs[i] = m_UIComs.back();
			m_UIComs.pop_back();
			break;
		}
	}
}

physx::PxVec2 UIPanel::GetPos()
{
	return { m_Trans->GetTransform().p.x,m_Trans->GetTransform().p.y };
}

void UIPanel::SetMovedPanel()
{
	m_Render->AddPixelFunc(std::bind(&UIPanelController::MovePanel, &s_PanelController, this),
		DirectX::Mouse::ButtonStateTracker::HELD, DirectX::MOUSEBUTTONINDEX::LEFTBUTTON);
}

void UIPanel::SetBackGroundColor(const physx::PxVec4& color)
{
	RenderInfo& info = m_Render->GetRenderInfo();
	info.meshOrTextureName = "";
	info.uiInfo.color = color;
}

void UIPanel::SetSize(const physx::PxVec2& size)
{
	m_Size = size;

	RenderInfo& info = m_Render->GetRenderInfo();
	info.uiInfo.size = m_Size;
}

void UIPanel::SetName(const std::wstring& name)
{
	m_Font->m_Text = name;
}

void UIPanel::SetPos(const physx::PxVec2& pos)
{
	m_Trans->SetPosX(pos.x - m_Size.x * m_BenchUV.x);
	m_Trans->SetPosY(pos.y - m_Size.y * m_BenchUV.y);
}

void UIPanel::SetPos(const physx::PxVec3& pos)
{
	m_Trans->SetPosX(pos.x - m_Size.x * m_BenchUV.x);
	m_Trans->SetPosY(pos.y - m_Size.y * m_BenchUV.y);
	m_Trans->SetPosZ(pos.z);
}

void UIPanel::Update(float delta)
{
	physx::PxVec3 comPos = m_Trans->GetTransform().p;
	comPos.x += CGH::GO.ui.panelComponentsInterval;
	comPos.z -= 0.001f;
	float topY = comPos.y;
	m_Font->m_Pos.x = comPos.x;
	m_Font->m_Pos.y = comPos.y + (CGH::GO.ui.panelTitleHeight-CGH::GO.ui.panelTitleTextHeight)/2.0f;
	m_Font->m_Pos.z = comPos.z;

	comPos.y += CGH::GO.ui.panelTitleHeight + CGH::GO.ui.panelComponentsInterval;

	physx::PxVec2 comSize;
	float mustX = 0;
	for (size_t i = 0; i < m_UIComs.size(); i++)
	{
		comSize = m_UIComs[i]->GetSize();

		m_UIComs[i]->SetPos(comPos);

		comPos.y += comSize.y + CGH::GO.ui.panelComponentsInterval;

		if (comSize.x > mustX)
		{
			mustX = comSize.x;
		}
	}

	mustX += CGH::GO.ui.panelComponentsInterval * 2;
	comPos.y += CGH::GO.ui.panelComponentsInterval;

	//#TODO Scroll.
	float currHeight = comPos.y - (comSize.y / 2) - topY;
	if (m_Size.y < currHeight || m_Size.x < mustX)
	{
		SetSize(physx::PxVec2(m_Size.x < mustX ? mustX : m_Size.x,
			m_Size.y < currHeight ? currHeight : m_Size.y));
	}

	auto& render = m_Render->GetRenderInfo();
	render.uiInfo.size = m_Size;
}

void UIPanel::UIPanelController::AddPanel(UIPanel* panel)
{
	m_Panels.push_back(panel);
}

void UIPanel::UIPanelController::DeletedPanel(UIPanel* panel)
{
	for (auto iter = m_Panels.begin(); iter != m_Panels.end(); iter++)
	{
		if ((*iter) == panel)
		{
			iter = m_Panels.erase(iter);
			break;
		}
	}
}

void UIPanel::UIPanelController::SortPanels(UIPanel* currPanel)
{
	bool isChanged = false;

	for (auto it = m_Panels.begin(); it != m_Panels.end(); it++)
	{
		if (currPanel == *it)
		{
			m_Panels.push_front(*it);
			it = m_Panels.erase(it);
			isChanged = true;
			break;
		}
	}

	if (isChanged)
	{
		float posZ = 0.5f;

		for (auto& it : m_Panels)
		{
			posZ += 0.01f;
			it->GetComponent<DOTransform>()->SetPosZ(posZ);
		}
	}

	m_CurrPanel = currPanel;
}

void UIPanel::UIPanelController::MovePanel(UIPanel* currPanel)
{
	if (currPanel == m_CurrPanel)
	{
		physx::PxVec2 movedValue = GETMOUSEMOUVEDVALUE;
		currPanel->SetPos(currPanel->GetPos() + movedValue);
	}
}
