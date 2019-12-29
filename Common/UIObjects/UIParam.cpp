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
	m_Font->SetFont(RenderFont::fontNames.front());
	m_Font->SetBenchmark(RenderFont::FONTBENCHMARK::LEFT);

	if (m_Type == UIPARAMTYPE::MODIFIER)
	{
		m_UICollision = AddComponent<ComUICollision>();
		m_UICollision->AddFunc(std::bind(&UIParam::SetUIParamToController, this));
	}
}

void UIParam::Update()
{
	if (m_ParamPtr)
	{
		physx::PxVec3 pos = m_Trans->GetTransform().p;

		m_Font->m_Pos.x = pos.x;
		m_Font->m_Pos.y = pos.y;
		m_Font->m_Pos.z = pos.z;

		std::wstring text = m_ParamName + L" : ";

		switch (m_Type)
		{
		case UIParam::UIPARAMTYPE::VIEWER:
		{
			text += GetDataString();
		}
			break;
		case UIParam::UIPARAMTYPE::MODIFIER:
		{
			DirectX::XMFLOAT2 fontDrawSize = m_Font->m_DrawSize;
			m_UICollision->SetSize({ fontDrawSize.x / 2, fontDrawSize.y / 2 });
			m_UICollision->SetOffset({ fontDrawSize.x / 2, 0 });

			if (!m_Selected)
			{
				text += GetDataString();
			}
			else
			{
				text += m_Font->m_Text;
			}
		}
			break;
		default:
			break;
		}

		m_Font->m_Text = text;
	}
}

void UIParam::SetUIParamToController()
{
	s_ParamController.SetUIParam(this);
}

std::wstring UIParam::GetDataString()
{
	std::wstring result;

	switch (m_DataType)
	{
	case CGH::DATA_TYPE::TYPE_BOOL:
		result = *reinterpret_cast<bool*>(m_ParamPtr) ? L"true" : L"false";
		break;
	case CGH::DATA_TYPE::TYPE_FLOAT:
		result = GetStringFromValue<float>();
		break;
	case CGH::DATA_TYPE::TYPE_INT:
		result = GetStringFromValue<int>();
		break;
	case CGH::DATA_TYPE::TYPE_UINT:
		result = GetStringFromValue<unsigned int>();
		break;
	default:
		assert(false);
		break;
	}

	return result;
}

void UIParam::ParamController::Update()
{
	if (m_CurrParam)
	{
		if (GETKEY(m_CurrParam))
		{
			m_CurrParam->Selected(true);
			if (keyboard->IsKeyPressed(KEYState::Enter))
			{
				Excute();
				WorkClear();
				WorkEnd();
				return;
			}
			else if (keyboard->IsKeyPressed(KEYState::Back))
			{
				if (m_InputData.size())
				{
					m_InputData.pop_back();
				}
			}
			else if (keyboard->IsKeyPressed(KEYState::OemMinus))
			{
				if (m_InputData.size() == 0)
				{
					m_InputData += '-';
				}
			}
			else if (keyboard->IsKeyPressed(KEYState::OemPeriod))
			{
				m_InputData += '.';
			}
			else
			{
				for (auto key = KEYState::D0; key <= KEYState::D9; key = static_cast<KEYState>(key + 1))
				{
					if (keyboard->IsKeyPressed(key))
					{
						m_InputData += ('0' + key - KEYState::D0);
						break;
					}
				}
			}
		}
		else
		{
			WorkEnd();
			return;
		}

		m_CurrParam->m_Font->m_Text.clear();
		m_CurrParam->m_Font->m_Text.insert(m_CurrParam->m_Font->m_Text.end(), m_InputData.begin(), m_InputData.end());
	}
}

void UIParam::ParamController::WorkClear()
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
	WorkStart();

	if (uiParam && m_CurrParam != uiParam)
	{
		if (uiParam->m_ParamPtr)
		{
			if (m_CurrParam)
			{
				WorkClear();
			}

			m_CurrParam = uiParam;
			m_CurrParam->Selected(true);
			m_InputData.clear();
		}
	}
}
