#pragma once
#include "UIButton.h"
#include "UIParam.h"
#include "foundation/PxVec2.h"

class UIPanel :public GameObject
{
private:
	enum class UICOMTYPE
	{
		UIBUTTON,
		UIPARAM
	};

	struct UIComObjects
	{
		UIComObjects(UICOMTYPE _type, GameObject* _object)
		{
			type = _type;
			object = _object;
		}
		UICOMTYPE type;
		GameObject* object;
	};

public:
	UIPanel(CGHScene& scene)
		:GameObject(scene)
		, m_Trans(nullptr)
		, m_Font(nullptr)
		, m_Render(nullptr)
		, m_UICollision(nullptr)
		, m_Active(true)
		, m_Size(10,10)
	{

	}
	virtual ~UIPanel() = default;

	void AddUICom(unsigned int x, unsigned y, UIButton* button);
	void AddUICom(unsigned int x, unsigned y, UIParam* param);

	void SetBackGroundTexture(const std::string & name);
	void SetBackGroundColor(DirectX::XMFLOAT4 color);
	void SetSize(unsigned int x, unsigned y);
	void SetName(const std::wstring& name);
	void SetPos(DirectX::XMFLOAT3 pos);
	void UIOn();
	void UIOff();

private:
	virtual void Init() override;
	virtual void Update() override;

private:
	bool			m_Active;
	ComTransform*	m_Trans;
	ComFont*		m_Font;
	ComRenderer*	m_Render;
	ComUICollision* m_UICollision;
	UIButton*		m_UIOffButton;

	physx::PxVec2	m_Size;
	std::vector<UIComObjects>	m_UIComs;
	std::vector<physx::PxVec2>	m_UIComOffset;
};