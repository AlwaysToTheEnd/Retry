#pragma once
#include "GameObject.h"
#include "BaseClass.h"
#include "StaticObject.h"
#include <DirectXMath.h>
#include <string>
#include <memory>

class ComFont;
class ComUICollision;
class ComTransform;

class UIParam :public GameObject
{
private:
	static class ParamController:public StaticObject
	{
	public:
		ParamController()
			:m_CurrParam(nullptr)
		{

		}
		virtual ~ParamController() = default;

		void SetUIParam(UIParam* uiParam);
	private:
		virtual void Init() override;
		virtual void Update() override;

	private:
		UIParam* m_CurrParam;

	} s_ParamController;

public:
	UIParam(CGHScene& scene)
		: GameObject(scene)
		, m_Font(nullptr)
		, m_UICollision(nullptr)
		, m_ParamPtr(nullptr)
		, m_DataType(CGH::DATA_TYPE::TYPE_INT)
	{

	}
	virtual ~UIParam() = default;

	template<typename T> void SetTargetParam(const std::wstring& paramName, T* data);
	void SetTextHeight(int height);

private:
	virtual void Init() override;
	virtual void Update() override;

private:
	std::wstring	m_ParamName;
	ComTransform*	m_Trans;
	ComFont*		m_Font;
	ComUICollision* m_UICollision;
	void*			m_ParamPtr;
	CGH::DATA_TYPE	m_DataType;
};

template<typename T>
inline void UIParam::SetTargetParam(const std::wstring& paramName, T* data)
{
	if (typeid(T) == typeid(int))
	{
		m_DataType = CGH::DATA_TYPE::TYPE_INT;
	}
	else if (typeid(T) == typeid(float))
	{
		m_DataType = CGH::DATA_TYPE::TYPE_FLOAT;
	}
	else if (typeid(T) == typeid(bool))
	{
		m_DataType = CGH::DATA_TYPE::TYPE_BOOL;
	}
	else if (typeid(T) == typeid(unsigned int))
	{
		m_DataType = CGH::DATA_TYPE::TYPE_UINT;
	}
	else
	{
		assert(false);
		return;
	}

	m_ParamName = paramName;
	m_ParamPtr = reinterpret_cast<void*>(data);
}

