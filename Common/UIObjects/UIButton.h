#pragma once
#include "UIObject.h"
#include <string>
#include <functional>

class DOFont;
class DOTransform;
class DORenderer;
class DOUICollision;

class UIButton :public UIObject
{
public:
	UIButton(CGHScene& scene, GameObject* parent, const char* typeName)
		: UIObject(scene, parent, typeName)
	{
	}
	virtual ~UIButton() = default;

	virtual void	SetPos(const physx::PxVec3& pos) override;
	void			SetTexture(const std::string& name, const physx::PxVec2& halfSize, bool colliderSizeIsEqualTexture = true);
	void			SetText(const std::wstring& text);
	void			SetColliderSize(const physx::PxVec2& halfSize);
	void			SetTextHeight(int height);

	void			AddFunc(std::function<void()> func);
	void			OnlyFontMode();

private:
	virtual void Init() override;
	virtual void Update(float delta) override;

private:
	bool			m_isOnlyFontMode;
	DOTransform*	m_Trans;
	DOFont*			m_Font;
	DORenderer*		m_Render;
	DOUICollision*	m_UICollision;
};