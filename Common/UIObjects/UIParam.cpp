#include "UIParam.h"
#include "UIPanel.h"
#include "UIButton.h"
#include "GraphicComponent.h"
#include "BaseComponent.h"
#include "d3dApp.h"

UIParam::ParamController UIParam::s_ParamController;

void UIParam::Delete()
{
	if (m_Selected)
	{
		s_ParamController.WorkClear();
	}

	GameObject::Delete();
}

void UIParam::SetEnumParam(const std::wstring& paramName, const std::vector<ENUM_ELEMENT>* elementInfo, int* data)
{
	m_ParamName = paramName;
	m_ParamPtr = reinterpret_cast<void*>(data);
	m_EnumElementInfo = elementInfo;
}

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

void UIParam::Update(unsigned long long delta)
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
			physx::PxVec2 fontDrawSize = m_Font->m_DrawSize;
			m_UICollision->SetSize({ fontDrawSize.x / 2, fontDrawSize.y / 2 });
			m_UICollision->SetOffset({ fontDrawSize.x / 2, 0 });

			if (!m_Selected)
			{
				text += GetDataString();
				m_Font->m_Color = { 0,0,0,1 };
			}
			else
			{
				text += m_Font->m_Text;
				m_Font->m_Color = { 1,0,0,1 };
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

	if (m_EnumElementInfo)
	{
		int paramValue = *reinterpret_cast<int*>(m_ParamPtr);
		bool isNotValidValue = true;

		for (auto& it : *m_EnumElementInfo)
		{
			if (it.value == paramValue)
			{
				result = it.elementName + L"(" + std::to_wstring(paramValue) + L")";
				isNotValidValue = false;
				break;
			}
		}

		if (isNotValidValue)
		{
			auto& element = m_EnumElementInfo->front();
			*reinterpret_cast<int*>(m_ParamPtr) = element.value;
			result = element.elementName + L"(" + std::to_wstring(element.value) + L")";
		}
	}
	else
	{
		switch (m_DataType)
		{
		case CGH::DATA_TYPE::TYPE_BOOL:
			result = *(reinterpret_cast<bool*>(m_ParamPtr)) ? L"true" : L"false";
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
	}

	return result;
}

void UIParam::ParamController::Update(unsigned long long delta)
{
	if (m_CurrParam)
	{
		if (GETKEY(m_CurrParam->GetConstructor()))
		{
			m_CurrParam->Selected(true);

			if (m_CurrParam->m_EnumElementInfo)
			{

			}
			else
			{
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
		}
		else
		{
			WorkClear();
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

	if (m_EnumSelectPanel)
	{
		m_EnumSelectPanel->UIOff();
	}
}

void UIParam::ParamController::Excute()
{
	if (m_InputData.size())
	{
		switch (m_CurrParam->m_DataType)
		{
		case CGH::DATA_TYPE::TYPE_BOOL:
			*reinterpret_cast<bool*>(m_CurrParam->m_ParamPtr) = atoi(m_InputData.c_str());
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

			if (m_CurrParam->m_EnumElementInfo)
			{
				CreateEnumPanel(m_CurrParam);
			}
		}
	}
}


void UIParam::ParamController::CreateEnumPanel(UIParam* param)
{
	if (m_EnumSelectPanel == nullptr)
	{
		m_EnumSelectPanel = param->CreateGameObject<UIPanel>(true);
		m_EnumSelectPanel->SetBackGroundTexture(InputTN::Get("UIParamEnumPanel"));
	}

	m_EnumSelectPanel->DeleteAllComs();
	m_EnumSelectPanel->UIOn();
	m_EnumSelectPanel->SetPos(GETAPP->GetMousePos());
	m_EnumSelectPanel->GetComponent<ComTransform>()->SetPosZ(0.1f);

	const int propertyIntervale = 15;
	int posY = 10;

	for (auto& it : *param->m_EnumElementInfo)
	{
		auto button = m_EnumSelectPanel->CreateGameObject<UIButton>(true);
		button->SetText(it.elementName);
		button->SetTextHeight(10);
		button->OnlyFontMode();
		button->AddFunc(std::bind(&UIParam::ParamController::SetEnumData, this, it.value));

		m_EnumSelectPanel->AddUICom(60, posY, button);
		posY += propertyIntervale;
	}

	m_EnumSelectPanel->SetSize(120, posY + propertyIntervale);
}

void UIParam::ParamController::SetEnumData(int value)
{
	*reinterpret_cast<int*>(m_CurrParam->m_ParamPtr) = value;
	WorkClear();
}
