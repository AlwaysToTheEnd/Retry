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
	m_UICollision = CreateComponenet<DOUICollision>();
	m_Render = CreateComponenet<DORenderer>();

	m_Trans->SetPosZ(0.8f);
	m_Font->SetFont(RenderFont::fontNames.front());
	m_Font->m_FontHeight = CGH::GO.ui.panelTitleTextHeight;
	RenderInfo info(RENDER_UI);
	info.point.size = { 100,100,0 };
	m_Render->SetRenderInfo(info);
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

		if (ui->Is(typeid(UIPanel).name()))
		{
			reinterpret_cast<UIPanel*>(ui)->ThisPanalIsStatic();
		}
	}
}

void UIPanel::DeleteAllComs()
{
	for (auto& it : m_UIComs)
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

void UIPanel::SetBackGroundTexture(const std::string& name)
{
	RenderInfo info = m_Render->GetRenderInfo();
	info.meshOrTextureName = name;
	m_Render->SetRenderInfo(info);
}

void UIPanel::SetBackGroundColor(const physx::PxVec4& color)
{
	RenderInfo info = m_Render->GetRenderInfo();
	info.meshOrTextureName = "";
	info.point.color = color;
	m_Render->SetRenderInfo(info);
}

void UIPanel::SetSize(const physx::PxVec2& size)
{
	m_Size = size;
	auto halfSize = m_Size / 2;

	RenderInfo info = m_Render->GetRenderInfo();
	info.point.size = { halfSize.x, halfSize.y,0 };
	if (m_UICollision)
	{
		m_UICollision->SetSize({ halfSize.x, halfSize.y });
	}
	m_Render->SetRenderInfo(info);
}

void UIPanel::SetName(const std::wstring& name)
{
	m_Font->m_Text = name;
}

void UIPanel::SetPos(const physx::PxVec2& pos)
{
	physx::PxVec2 uv = m_BenchUV - physx::PxVec2(0.5f, 0.5f);
	m_Trans->SetPosX(pos.x - m_Size.x * uv.x);
	m_Trans->SetPosY(pos.y - m_Size.y * uv.y);
}

void UIPanel::SetPos(const physx::PxVec3& pos)
{
	physx::PxVec2 uv = m_BenchUV - physx::PxVec2(0.5f, 0.5f);
	m_Trans->SetPosX(pos.x - m_Size.x * uv.x);
	m_Trans->SetPosY(pos.y - m_Size.y * uv.y);
	m_Trans->SetPosZ(pos.z);
}

void UIPanel::ThisPanalIsStatic()
{
	if (m_UICollision)
	{
		ExceptComponent(m_UICollision);
		m_UICollision = nullptr;
	}
}

void UIPanel::Update(float delta)
{
	physx::PxVec3 comPos = m_Trans->GetTransform().p;
	auto halfSize = m_Size / 2;
	comPos.x -= halfSize.x;
	comPos.y -= halfSize.y;
	comPos.z -= 0.001f;
	float topY = comPos.y;
	m_Font->m_Pos.x = comPos.x;
	m_Font->m_Pos.y = comPos.y + m_Font->m_FontHeight / 2.0f;
	m_Font->m_Pos.z = comPos.z;

	comPos.y += m_ComsInterval + m_TitleSize;

	physx::PxVec2 comSize;
	float mustX = 0;
	for (size_t i = 0; i < m_UIComs.size(); i++)
	{
		comSize = m_UIComs[i]->GetSize();

		m_UIComs[i]->SetPos(comPos);

		comPos.y += comSize.y + m_ComsInterval;

		if (comSize.x > mustX)
		{
			mustX = comSize.x;
		}
	}
	//#TODO Scroll.
	float currHeight = comPos.y - (comSize.y / 2) - topY;
	if (m_Size.y < currHeight || m_Size.x < mustX)
	{
		SetSize(physx::PxVec2(m_Size.x < mustX ? mustX : m_Size.x,
			m_Size.y < currHeight ? currHeight : m_Size.y));
	}
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

void UIPanel::UIPanelController::Update(float delta)
{
	if (m_CurrPanel)
	{
		if (GETMOUSE(m_CurrPanel->GetConstructor()))
		{
			auto mouseState = mouse->GetLastState();
			physx::PxVec2 mousePos = physx::PxVec2(mouseState.x, mouseState.y);
			m_PressedTime += delta;

			auto state = m_CurrPanel->GetClickedState();
			switch (state)
			{
			case GameObject::CLICKEDSTATE::HELD:
			{
				physx::PxVec2 moveValue = mousePos - m_PrevMousePos;
				m_CurrPanel->GetComponent<DOTransform>()->AddVector({ moveValue.x,moveValue.y,0 });
				m_PrevMousePos = mousePos;
			}
			break;
			case GameObject::CLICKEDSTATE::NONE:
			case GameObject::CLICKEDSTATE::MOUSEOVER:
			case GameObject::CLICKEDSTATE::RELEASED:
				WorkClear();
				break;
			default:
				break;
			}

			if (mouse->leftButton == MOUSEState::RELEASED)
			{
				if (m_PressedTime > 0.1f)
				{
					HOLDCANCLE(m_CurrPanel->GetConstructor());
				}

				WorkClear();
			}
		}
	}
	else
	{
		for (auto& it : m_Panels)
		{
			if (it->GetClickedState() == GameObject::CLICKEDSTATE::PRESSED)
			{
				if (GETMOUSE(it->GetConstructor()))
				{
					auto mouseState = mouse->GetLastState();
					physx::PxVec2 mousePos = physx::PxVec2(mouseState.x, mouseState.y);

					m_PrevMousePos = mousePos;
					m_PressedTime = 0;
					m_CurrPanel = it;
					SortPanels(it);
				}

				break;
			}
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
}

void UIPanel::UIPanelController::WorkClear()
{
	if (m_CurrPanel)
	{
		m_CurrPanel = nullptr;
	}

	m_PressedTime = 0;
}
