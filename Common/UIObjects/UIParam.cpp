#include "UIParam.h"
#include "GraphicComponent.h"
#include "BaseComponent.h"
#include "d3dApp.h"

UIParam::ParamController UIParam::s_ParamController;

void UIParam::SetTextHeight(int height)
{
	m_Font->m_FontHeight = height;
}

void UIParam::Init()
{
	m_Trans = AddComponent<ComTransform>();
	m_Font = AddComponent<ComFont>();
	m_UICollision = AddComponent<ComUICollision>();

	m_Font->SetFont(RenderFont::fontNames.front());
	m_UICollision->AddFunc(std::bind(&UIParam::SetUIParamToController, this));
}

void UIParam::Update()
{
	if (m_ParamPtr)
	{
		physx::PxVec3 pos = m_Trans->GetTransform().p;

		m_Font->m_OffsetPos.x = pos.x;
		m_Font->m_OffsetPos.y = pos.y;

		DirectX::XMFLOAT2 halfSize = m_Font->m_DrawSize;
		m_UICollision->SetSize({ halfSize.x / 2, halfSize.y / 2 });

		std::wstring text = m_ParamName + L" : ";

		if (!m_Selected)
		{
			switch (m_DataType)
			{
			case CGH::DATA_TYPE::TYPE_BOOL:
				text += *reinterpret_cast<bool*>(m_ParamPtr) ? L"true" : L"false";
				break;
			case CGH::DATA_TYPE::TYPE_FLOAT:
				text += GetStringFromValue<float>();
				break;
			case CGH::DATA_TYPE::TYPE_INT:
				text += GetStringFromValue<int>();
				break;
			case CGH::DATA_TYPE::TYPE_UINT:
				text += GetStringFromValue<unsigned int>();
				break;
			default:
				assert(false);
				break;
			}
		}
		else
		{
			text += m_Font->m_Text;
		}

		m_Font->m_Text = text;
	}
}

void UIParam::SetUIParamToController()
{
	s_ParamController.SetUIParam(this);
}

void UIParam::ParamController::Init()
{

}

void UIParam::ParamController::Update()
{
	if (m_CurrParam)
	{
		auto keyboard = GETAPP->GetKeyBoard();

		if (keyboard.IsKeyPressed(KEYState::Escape))
		{
			Clear();
			return;
		}
		else if (keyboard.IsKeyPressed(KEYState::Enter))
		{
			Excute();
			Clear();
			return;
		}
		else if (keyboard.IsKeyPressed(KEYState::Back))
		{
			if (m_InputData.size())
			{
				m_InputData.pop_back();
			}
		}
		else if (keyboard.IsKeyPressed(KEYState::OemMinus))
		{
			if (m_InputData.size() == 0)
			{
				m_InputData += '-';
			}
		}
		else if (keyboard.IsKeyPressed(KEYState::OemPeriod))
		{
			m_InputData += '.';
		}
		else
		{
			for (auto key = KEYState::D0; key <= KEYState::D9; key = static_cast<KEYState>(key + 1))
			{
				if (keyboard.IsKeyPressed(key))
				{
					m_InputData += ('0' + key - KEYState::D0);
					break;
				}
			}
		}

		m_CurrParam->m_Font->m_Text.clear();
		m_CurrParam->m_Font->m_Text.insert(m_CurrParam->m_Font->m_Text.end(), m_InputData.begin(), m_InputData.end());
	}
}

void UIParam::ParamController::Clear()
{
	m_InputData.clear();

	if (m_CurrParam)
	{
		m_CurrParam->Selected(false);
		m_CurrParam = nullptr;
	}
}

void UIParam::ParamController::Excute()
{
	if (m_InputData.size())
	{
		switch (m_CurrParam->m_DataType)
		{
		case CGH::DATA_TYPE::TYPE_BOOL:
			*reinterpret_cast<bool*>(m_CurrParam->m_ParamPtr) = m_InputData != "0";
			break;
		case CGH::DATA_TYPE::TYPE_FLOAT:
			*reinterpret_cast<float*>(m_CurrParam->m_ParamPtr) = atof(m_InputData.c_str());
			break;
		case CGH::DATA_TYPE::TYPE_INT:
			*reinterpret_cast<int*>(m_CurrParam->m_ParamPtr) = atoi(m_InputData.c_str());
			break;
		case CGH::DATA_TYPE::TYPE_UINT:
			*reinterpret_cast<UINT*>(m_CurrParam->m_ParamPtr) = atoi(m_InputData.c_str());
			break;
		default:
			break;
		}
	}
}

void UIParam::ParamController::SetUIParam(UIParam* uiParam)
{
	if (uiParam && m_CurrParam != uiParam)
	{
		if (uiParam->m_ParamPtr)
		{
			if (m_CurrParam)
			{
				Clear();
			}

			m_CurrParam = uiParam;
			m_CurrParam->Selected(true);
			m_InputData.clear();
		}
	}
}
