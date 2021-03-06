#pragma once
#include "DeviceObject.h"
#include <vector>
#include <memory>
#include <assert.h>

template <typename T>
class InstanceAndIndexManager
{
public:
	virtual ~InstanceAndIndexManager()=default;

	unsigned int GetNextID();
	void AddData(T com);
	void AddData();
	virtual void SignalDeleted(unsigned int id);
	virtual T& GetData(unsigned int id) { return m_Datas[id]; }

protected:
	std::vector<T>				m_Datas;
	std::vector<unsigned int>	m_DeletedIndices;
};

class DeviceObjectUpdater : public InstanceAndIndexManager<DeviceObject*>
{
public:
	virtual ~DeviceObjectUpdater();

	void Update(float delta);
	virtual void SignalDeleted(unsigned int id) override;
};

template<typename T>
inline unsigned int InstanceAndIndexManager<T>::GetNextID()
{
	unsigned int index = 0;

	if (m_DeletedIndices.size())
	{
		index = m_DeletedIndices.back();
	}
	else
	{
		index = CGH::SizeTTransUINT(m_Datas.size());
	}

	return index;
}

template<typename T>
inline void InstanceAndIndexManager<T>::AddData(T com)
{
	if (m_DeletedIndices.size())
	{
		m_Datas[m_DeletedIndices.back()] = std::move(com);
		m_DeletedIndices.pop_back();
	}
	else
	{
		m_Datas.push_back(com);
	}
}

template<typename T>
inline void InstanceAndIndexManager<T>::AddData()
{
	if (m_DeletedIndices.size())
	{
		m_DeletedIndices.pop_back();
	}
	else
	{
		m_Datas.emplace_back();
	}
}

template<typename T>
inline void InstanceAndIndexManager<T>::SignalDeleted(unsigned int id)
{
	assert(id <= m_Datas.size());

	m_DeletedIndices.push_back(id);
}
