#include "UIParam.h"
#include "GraphicComponent.h"
#include "BaseComponent.h"

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

		std::wstring text = m_ParamName+L" : ";

		switch (m_DataType)
		{
		case CGH::DATA_TYPE::TYPE_BOOL:
			text += *reinterpret_cast<bool*>(m_ParamPtr) ? L"true" : L"false";
			break;
		case CGH::DATA_TYPE::TYPE_FLOAT:
			text += std::to_wstring(*reinterpret_cast<float*>(m_ParamPtr));
			break;
		case CGH::DATA_TYPE::TYPE_INT:
			text += std::to_wstring(*reinterpret_cast<int*>(m_ParamPtr));
			break;
		case CGH::DATA_TYPE::TYPE_UINT:
			text += std::to_wstring(*reinterpret_cast<unsigned int*>(m_ParamPtr));
			break;
		default:
			assert(false);
			break;
		}
		
		m_Font->m_Text = text;
	}
}

void UIParam::ParamController::Init()
{

}

void UIParam::ParamController::Update()
{

}

void UIParam::ParamController::SetUIParam(UIParam* uiParam)
{
	m_CurrParam = uiParam;
}
