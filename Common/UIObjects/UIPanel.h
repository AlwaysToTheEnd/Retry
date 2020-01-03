#pragma once
#include "UIButton.h"
#include "UIParam.h"
#include "foundation/PxVec2.h"
#include <list>

class UIPanel :public GameObject
{
private:
	static class UIPanelController :public StaticGameObjectController
	{
	public:
		UIPanelController()
			: StaticGameObjectController(true)
			, m_CurrPanel(nullptr)
			, m_PressedTime(0)
			, m_PrevMousePos(0,0)
		{

		}
		virtual ~UIPanelController() = default;

		void AddPanel(UIPanel* panel);
		void DeletedPanel(UIPanel* panel);

		virtual void WorkClear() override;
	private:
		virtual void Update(float delta) override;
		void SortPanels(UIPanel* currPanel);

	private:
		UIPanel*			m_CurrPanel;
		std::list<UIPanel*>	m_Panels;
		physx::PxVec2		m_PrevMousePos;
		float				m_PressedTime;

	} s_PanelController;

private:
	enum class UICOMTYPE
	{
		UIBUTTON,
		UIPARAM,
		UIPANEL,
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
		, m_Size(10, 10)
	{
		s_PanelController.AddPanel(this);
	}

	virtual ~UIPanel() = default;
	virtual void Delete() override;

	void AddUICom(unsigned int x, unsigned y, UIButton* button);
	void AddUICom(unsigned int x, unsigned y, UIParam* param);
	void AddUICom(unsigned int x, unsigned y, UIPanel* panel);
	void DeleteAllComs();

	physx::PxVec2 GetSize() { return m_Size; }
	physx::PxVec2 GetPos();
	unsigned int GetNumAddedComs() { return m_UIComs.size(); }
	void SetBackGroundTexture(const std::string& name);
	void SetBackGroundColor(const physx::PxVec4& color);
	void SetSize(unsigned int x, unsigned y);
	void SetName(const std::wstring& name);
	void SetPos(const physx::PxVec2& pos);
	void UIOn();
	void UIOff();
	void ThisPanalIsStatic();

private:
	virtual void Init() override;
	virtual void Update(float delta) override;

private:
	bool			m_Active;
	ComTransform*	m_Trans;
	ComFont*		m_Font;
	ComRenderer*	m_Render;
	ComUICollision* m_UICollision;

	physx::PxVec2	m_Size;
	std::vector<UIComObjects>	m_UIComs;
	std::vector<physx::PxVec2>	m_UIComOffset;
};