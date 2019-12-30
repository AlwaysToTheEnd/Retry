#include "UIPanel.h"
#include "BaseComponent.h"
#include "GraphicComponent.h"
#include "CGHScene.h"
#include "d3dApp.h"

UIPanel::UIPanelController UIPanel::s_PanelController;

void UIPanel::Init()
{
	m_Trans = AddComponent<ComTransform>();
	m_Font = AddComponent<ComFont>();
	m_UICollision = AddComponent<ComUICollision>();
	m_Render = AddComponent<ComRenderer>();

	m_Trans->SetPosZ(0.9f);
	m_Font->SetFont(RenderFont::fontNames.front());
	m_Font->m_FontHeight = 10;
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

void UIPanel::AddUICom(unsigned int x, unsigned y, UIButton* button)
{
	m_UIComs.push_back({ UICOMTYPE::UIBUTTON, button });
	m_UIComOffset.push_back({ static_cast<float>(x),static_cast<float>(y) });
	button->SetConstructor(GetConstructor());
}

void UIPanel::AddUICom(unsigned int x, unsigned y, UIParam* param)
{
	m_UIComs.push_back({ UICOMTYPE::UIPARAM, param });
	m_UIComOffset.push_back({ static_cast<float>(x),static_cast<float>(y) });
	param->SetConstructor(GetConstructor());
}

void UIPanel::AddUICom(unsigned int x, unsigned y, UIPanel* panel)
{
	m_UIComs.push_back({ UICOMTYPE::UIPANEL, panel });
	m_UIComOffset.push_back({ static_cast<float>(x),static_cast<float>(y) });
	panel->SetConstructor(GetConstructor());

	panel->ThisPanalIsStatic();
}

void UIPanel::DeleteAllComs()
{
	for (auto& it : m_UIComs)
	{
		it.object->Delete();
	}

	m_UIComOffset.clear();
	m_UIComs.clear();
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

void UIPanel::SetPos(DirectX::XMFLOAT2 pos)
{
	m_Trans->SetPosX(pos.x);
	m_Trans->SetPosY(pos.y);
	
}

void UIPanel::UIOn()
{
	m_Active = true;

	SetAllComponentActive(true);

	for (auto& it : m_UIComs)
	{
		it.object->SetAllComponentActive(true);
	}
}

void UIPanel::UIOff()
{
	m_Active = false;

	SetAllComponentActive(false);

	for (auto& it : m_UIComs)
	{
		it.object->SetAllComponentActive(false);
	}
}

void UIPanel::ThisPanalIsStatic()
{
	if (m_UICollision)
	{
		DeleteComponent(m_UICollision);
		m_UICollision = nullptr;
	}
}

void UIPanel::Update()
{
	if (m_Active)
	{
		physx::PxTransform panelTransform = m_Trans->GetTransform();
		auto halfSize = m_Size / 2;
		m_Font->m_Pos.x = panelTransform.p.x -halfSize.x;
		m_Font->m_Pos.y = panelTransform.p.y -halfSize.y+ m_Font->m_FontHeight/2.0f;
		m_Font->m_Pos.z = panelTransform.p.z - 0.001f;

		for (size_t i = 0; i < m_UIComs.size(); i++)
		{
			auto transform = m_UIComs[i].object->GetComponent<ComTransform>();

			transform->SetTransform(
				physx::PxTransform(m_UIComOffset[i].x - halfSize.x, m_UIComOffset[i].y - halfSize.y, -0.001f)
				* panelTransform);
		}
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

void UIPanel::UIPanelController::Update()
{
	if (m_CurrPanel)
	{
		if (GETMOUSE(m_CurrPanel->GetConstructor()))
		{
			auto mouseState = mouse->GetLastState();
			physx::PxVec2 mousePos = physx::PxVec2(mouseState.x, mouseState.y);
			static std::vector<UIPanel*> test;

			if (test.size() == 0)
			{
				test.push_back(m_CurrPanel);
			}
			else
			{
				if (test.back() != m_CurrPanel)
				{
					test.push_back(m_CurrPanel);
				}
			}

			if (mouse->leftButton == MOUSEState::HELD)
			{
				m_PressedTime += 1;//#TODO GetDELTATIME
				physx::PxVec2 moveValue =  mousePos - m_PrevMousePos;
				m_CurrPanel->GetComponent<ComTransform>()->AddVector({ moveValue.x,moveValue.y,0 });
				m_PrevMousePos = mousePos;
			}
			else if (mouse->leftButton == MOUSEState::RELEASED)
			{
				physx::PxVec2 moveValue = mousePos - m_PrevMousePos;
				m_CurrPanel->GetComponent<ComTransform>()->AddVector({ moveValue.x,moveValue.y,0 });
				m_PrevMousePos = mousePos;

				if (m_PressedTime > 100)
				{
					HOLDCANCLE(m_CurrPanel->GetConstructor());
				}

				WorkClear();
			}
		}
	}
	else
	{
		bool isChanged = false;

		for (auto it = m_Panels.begin(); it != m_Panels.end(); it++)
		{
			if (GETMOUSE((*it)->GetConstructor()))
			{
				if (mouse->leftButton == MOUSEState::HELD && m_CurrPanel != *it)
				{
					auto mouseState = mouse->GetLastState();
					m_CurrPanel = *it;
					m_PrevMousePos = physx::PxVec2(mouseState.x, mouseState.y);

					m_Panels.push_front(*it);
					it = m_Panels.erase(it);
					isChanged = true;
				}

				break;
			}
		}

		if (isChanged)
		{
			float posZ = 0.1f;

			for (auto& it : m_Panels)
			{
				posZ += 0.01f;
				it->GetComponent<ComTransform>()->SetPosZ(posZ);
			}
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
