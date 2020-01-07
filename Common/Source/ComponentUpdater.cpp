#include "ComponentUpdater.h"

DeviceObjectUpdater::~DeviceObjectUpdater()
{
	for (auto& it : m_Datas)
	{
		if (it != nullptr)
		{
			delete it;
		}
	}

	m_Datas.clear();
}

void DeviceObjectUpdater::Update(float delta)
{
	for (auto& it : m_Datas)
	{
		if (it != nullptr)
		{
			if (it->GetActive())
			{
				it->Update(delta);
			}
		}
	}
}

void DeviceObjectUpdater::SignalDeleted(unsigned int id)
{
	assert(id <= m_Datas.size());

	delete m_Datas[id];
	m_Datas[id] = nullptr;
	m_DeletedIndices.push_back(id);
}
