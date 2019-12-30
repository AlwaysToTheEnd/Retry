#pragma once
#include <functional>
#include <vector>

class GameObject;
class D3DApp;

enum class COMPONENTTYPE
{
	COM_DYNAMIC,
	COM_STATIC,
	COM_UICOLLISTION,
	COM_TRANSFORM,

	COM_MESH,
	COM_ANIMATOR,
	COM_RENDERER,
	COM_FONT,
	COM_END,
};

class IComponent
{
public:
	static const int NUMCOMPONENTTYPE = static_cast<int>(COMPONENTTYPE::COM_END);
	friend class D3DApp;
public:
	IComponent(COMPONENTTYPE type, GameObject& gameObject, int ID);
	virtual ~IComponent();
	virtual void Update(unsigned long long delta) {};

	COMPONENTTYPE GetType() const { return m_Type; }
	void SetActive(bool value) { m_IsActive = value; }
	bool IsActive() const { return m_IsActive; }
	int GetID() const { return m_ID; }

protected:
	GameObject& const	m_TargetGameObject;

private:
	COMPONENTTYPE		m_Type;
	bool				m_IsActive;
	const int			m_ID;
};