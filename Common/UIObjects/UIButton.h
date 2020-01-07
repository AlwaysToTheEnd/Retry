#pragma once
#include "GameObject.h"
#include <string>
#include <functional>

class DOFont;
class DOTransform;
class DORenderer;
class DOUICollision;

class UIButton :public GameObject
{
public:
	UIButton(CGHScene& scene, GameObject* parent, const char* typeName)
		: GameObject(scene, parent, typeName)
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
	virtual void Update(float delta) override;

private:
	bool			m_isOnlyFontMode;
	DOTransform*	m_Trans;
	DOFont*			m_Font;
	DORenderer*		m_Render;
	DOUICollision*	m_UICollision;
};