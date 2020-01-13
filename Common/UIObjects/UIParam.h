#pragma once
#include "UIObject.h"
#include "BaseClass.h"
#include "StaticObject.h"
#include <DirectXMath.h>
#include <string>
#include <memory>

class DOFont;
class DOUICollision;
class DOTransform;
class UIPanel;

class UIParam :public UIObject
{
public:
	enum class UIPARAMTYPE
	{
		VIEWER,
		MODIFIER,
	};

	enum class UICONTROLTYPE
	{
		ORIGIN_DATA,
		ENUM_DATA,
		STRING_DATA
	};

private:
	static class ParamController:public StaticGameObjectController
	{
	public:
		ParamController()
			: StaticGameObjectController(false)
			, m_CurrParam(nullptr)
			, m_EnumSelectPanel(nullptr)
		{
		}
		virtual ~ParamController() = default;

		void SetUIParam(UIParam* uiParam);
		virtual void WorkClear() override;

	private:
		virtual void Update(float delta) override;
		void CreateSubPanel(UIParam* param);
		void SetEnumData(int value);
		void SetStringData(const std::string& str);
		void Excute();

	private:
		std::string		m_InputData;
		UIParam*		m_CurrParam;
		UIPanel*		m_EnumSelectPanel;

	} s_ParamController;

public:
	UIParam(CGHScene& scene, GameObject* parent, const char* typeName, UIPARAMTYPE type)
		: UIObject(scene, parent, typeName)
		, m_Type(type)
		, m_ControlType(UICONTROLTYPE::ORIGIN_DATA)
		, m_Font(nullptr)
		, m_Trans(nullptr)
		, m_UICollision(nullptr)
		, m_ParamPtr(nullptr)
		, m_DataType(CGH::DATA_TYPE::TYPE_INT)
		, m_Selected(false)
		, m_EnumElementInfo(nullptr)
		, m_Strings(nullptr)
	{
		
	}
	virtual			~UIParam() = default;
	virtual void	Delete() override;

	void						AddUIParam(UIParam* param);
	virtual void				SetPos(const physx::PxVec3& pos) override;
	template<typename T> void	SetTargetParam(const std::wstring& paramName, T* data);
	void						SetEnumParam(const std::wstring& paramName, const std::vector<ENUM_ELEMENT>* elementInfo, int* data);
	void						SetStringParam(const std::wstring& paramName, const std::vector<std::string>* strings, std::string* data);
	void						SetTextHeight(int height);
	void						SetDirtyCall(std::function<void()> dirtyCall);

private:
	virtual void	Init() override;
	virtual void	Update(float delta) override;
	void			SetUIParamToController();
	void			Selected(bool value) { m_Selected = value; }

	std::wstring						GetDataString();
	template<typename T> std::wstring	GetStringFromValue();

private:
	static const int	m_ParamInterval = 3;
	const UIPARAMTYPE	m_Type;
	UICONTROLTYPE		m_ControlType;
	std::wstring		m_ParamName;
	DOTransform*		m_Trans;
	DOFont*				m_Font;
	DOUICollision*		m_UICollision;
	void*				m_ParamPtr;
	CGH::DATA_TYPE		m_DataType;
	bool				m_Selected;

	const std::vector<ENUM_ELEMENT>*	m_EnumElementInfo;
	const std::vector<std::string>*		m_Strings;
	std::function<void()>				m_DirtyCall;
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

	m_ControlType = UICONTROLTYPE::ORIGIN_DATA;
	m_ParamName = paramName;
	m_ParamPtr = reinterpret_cast<void*>(data);
	m_EnumElementInfo = nullptr;
	m_DirtyCall = nullptr;
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

