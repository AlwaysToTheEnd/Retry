#pragma once
class ComTransform;
class GameObject;

enum class COMPONENTTYPE
{
	COM_PHYSICS,

	COM_TRANSFORM,

	COM_RENDERER,
	COM_MESH,
	COM_ANIMATER,
	COM_END
};

static const int NUMCOMPONENTTYPE = static_cast<int>(COMPONENTTYPE::COM_END);

class IComponent
{
public:
	IComponent(GameObject& gameObject, COMPONENTTYPE type)
		: m_TargetGameObject(&gameObject)
		, m_Type(type)
	{
	};
	virtual ~IComponent() = default;

	virtual void Update() = 0;
	COMPONENTTYPE GetType() { return m_Type; }

protected:
	COMPONENTTYPE	m_Type;
	bool			m_IsActive = true;
	GameObject*		m_TargetGameObject = nullptr;
};