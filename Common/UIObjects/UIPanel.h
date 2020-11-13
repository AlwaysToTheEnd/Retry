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
		{

		}
		virtual ~UIPanelController() = default;

		void			SortPanels(UIPanel* currPanel);
		void			MovePanel(UIPanel* currPanel);
		void			AddPanel(UIPanel* panel);
		void			DeletedPanel(UIPanel* panel);

		virtual void	WorkClear() override {};

	private:
		virtual void	Update(float delta) override {};

	private:
		std::list<UIPanel*>	m_Panels;
		UIPanel*			m_CurrPanel;

	} s_PanelController;

public:
	UIPanel(CGHScene& scene, GameObject* parent, const char* typeName)
		: UIObject(scene, parent, typeName)
		, m_Trans(nullptr)
		, m_Font(nullptr)
		, m_Render(nullptr)
	{
		s_PanelController.AddPanel(this);
	}

	virtual			~UIPanel() = default;
	virtual void	Delete() override;
	void			DeleteAllComs();
	void			PopUICom(const UIObject* uiCom);

	void			AddUICom(UIObject* ui);
	void			AddPixelFunc(std::function<void()> func,
					DirectX::Mouse::ButtonStateTracker::ButtonState state, DirectX::MOUSEBUTTONINDEX index);

	physx::PxVec2	GetPos();
	unsigned int	GetNumAddedComs() { return CGH::SizeTTransUINT(m_UIComs.size()); }

	void			SetMovedPanel();
	void			SetBackGroundColor(const physx::PxVec4& color);
	void			SetSize(const physx::PxVec2& size);
	void			SetName(const std::wstring& name);
	void			SetPos(const physx::PxVec2& pos);
	virtual void	SetPos(const physx::PxVec3& pos) override;

protected:
	virtual void Init() override;
	virtual void Update(float delta) override;

protected:
	DOTransform*			m_Trans;
	DOFont*					m_Font;
	DORenderer*				m_Render;

	std::vector<UIObject*>	m_UIComs;
};