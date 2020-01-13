#pragma once
#include "UIButton.h"
#include "UIParam.h"
#include "foundation/PxVec2.h"
#include <list>

class UIPanel :public UIObject
{
protected:
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

		void			AddPanel(UIPanel* panel);
		void			DeletedPanel(UIPanel* panel);

		virtual void	WorkClear() override;

	private:
		virtual void	Update(float delta) override;

		void			SortPanels(UIPanel* currPanel);

	private:
		UIPanel*			m_CurrPanel;
		std::list<UIPanel*>	m_Panels;
		physx::PxVec2		m_PrevMousePos;
		float				m_PressedTime;

	} s_PanelController;

public:
	UIPanel(CGHScene& scene, GameObject* parent, const char* typeName)
		: UIObject(scene, parent, typeName)
		, m_Trans(nullptr)
		, m_Font(nullptr)
		, m_Render(nullptr)
		, m_UICollision(nullptr)
	{
		s_PanelController.AddPanel(this);
	}

	virtual			~UIPanel() = default;
	virtual void	Delete() override;
	void			DeleteAllComs();
	void			PopUICom(const UIObject* uiCom);

	void			AddUICom(UIObject* ui);

	physx::PxVec2	GetSize() { return m_Size; }
	physx::PxVec2	GetPos();
	unsigned int	GetNumAddedComs() { return m_UIComs.size(); }

	void			SetBackGroundTexture(const std::string& name);
	void			SetBackGroundColor(const physx::PxVec4& color);
	void			SetSize(const physx::PxVec2& size);
	void			SetName(const std::wstring& name);
	void			SetPos(const physx::PxVec2& pos);
	virtual void	SetPos(const physx::PxVec3& pos) override;

	void			ThisPanalIsStatic();

protected:
	virtual void Init() override;
	virtual void Update(float delta) override;

protected:
	static const int		m_ComsInterval = 3;
	static const int		m_TitleSize = 20;
	DOTransform*			m_Trans;
	DOFont*					m_Font;
	DORenderer*				m_Render;
	DOUICollision*			m_UICollision;

	std::vector<UIObject*>	m_UIComs;
};