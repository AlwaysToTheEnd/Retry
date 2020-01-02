#pragma once
#include "GameObject.h"
#include <string>
#include <functional>

class ComFont;
class ComTransform;
class ComRenderer;
class ComUICollision;

class UIButton :public GameObject
{
public:
	UIButton(CGHScene& scene)
		:GameObject(scene)
	{
	}
	virtual ~UIButton() = default;

	void SetTexture(const std::string& name, const physx::PxVec2& halfSize, bool colliderSizeIsEqualTexture = true);
	void SetText(const std::wstring& text);
	void OnlyFontMode();
	void SetTextHeight(int height);
	void SetColliderSize(const physx::PxVec2& halfSize);
	void AddFunc(std::function<void()> func);

private:
	virtual void Init() override;
	virtual void Update(unsigned long long delta) override;

private:
	bool			m_isOnlyFontMode;
	ComTransform*	m_Trans;
	ComFont*		m_Font;
	ComRenderer*	m_Render;
	ComUICollision*	m_UICollision;
};