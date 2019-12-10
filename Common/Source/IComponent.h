#pragma once
#include <functional>

class GameObject;
class D3DApp;

enum class COMPONENTTYPE
{
	COM_PHYSICS,

	COM_TRANSFORM,

	COM_RENDERER,
	COM_MESH,
	COM_ANIMATER,
	COM_END,
};

extern const int NUMCOMPONENTTYPE = static_cast<int>(COMPONENTTYPE::COM_END);

class IComponent
{
	friend class D3DApp;
public:
	IComponent(COMPONENTTYPE type, GameObject& gameObject, int ID);
	~IComponent();
	virtual void Update() {};

	COMPONENTTYPE GetType() const { return m_Type; }
	bool IsActive() const { return m_IsActive; }
	int GetID() const { return m_ID; }

protected:
	GameObject*		m_TargetGameObject = nullptr;
	COMPONENTTYPE	m_Type;
	bool			m_IsActive = true;

private:
	int m_ID = -1;

private:
	static void SetInstanceDeleteManagingFunc(std::function<void(COMPONENTTYPE, int)> func);

	static std::function<void(COMPONENTTYPE, int)>	m_InstanceDeleteManagingFunc;
};