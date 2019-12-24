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
	static class ParamController:public StaticGameObjectController
	{
	public:
		ParamController()
			: m_CurrParam(nullptr)
		{

		}
		virtual ~ParamController() = default;

		void SetUIParam(UIParam* uiParam);
	private:
		virtual void Init() override;
		virtual void Update() override;
		virtual void WorkClear() override;
		void Excute();

	private:
		std::string		m_InputData;
		UIParam*		m_CurrParam;

	} s_ParamController;

public:
	UIParam(CGHScene& scene)
		: GameObject(scene)
		, m_Font(nullptr)
		, m_UICollision(nullptr)
		, m_ParamPtr(nullptr)
		, m_DataType(CGH::DATA_TYPE::TYPE_INT)
		, m_Selected(false)
	{

	}
	virtual ~UIParam() = default;

	template<typename T> void SetTargetParam(const std::wstring& paramName, T* data);
	void SetTextHeight(int height);

private:
	virtual void Init() override;
	virtual void Update() override;
	void SetUIParamToController();
	void Selected(bool value) { m_Selected = value; }

	template<typename T> std::wstring GetStringFromValue();
private:
	std::wstring	m_ParamName;
	ComTransform*	m_Trans;
	ComFont*		m_Font;
	ComUICollision* m_UICollision;
	void*			m_ParamPtr;
	CGH::DATA_TYPE	m_DataType;
	bool			m_Selected;
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

template<typename T>
inline std::wstring UIParam::GetStringFromValue()
{
	std::wstring result;

	result = std::to_wstring(*reinterpret_cast<T*>(m_ParamPtr));
	size_t period = result.find(L'.');

	if (period < result.size())
	{
		while (result.size())
		{
			if (result.back() == L'0')
			{
				if (period == result.size() - 2)
				{
					break;
				}

				result.pop_back();
			}
			else
			{

				break;
			}
		}
	}

	return result;
}

