#include "ComponentUpdater.h"

void DeviceObjectUpdater::Update(float delta)
{
	for (auto& it : m_Datas)
	{
		if (it != nullptr)
		{
			if (it->IsActive())
			{
				it->Update(delta);
			}
		}
	}
}

void DeviceObjectUpdater::SignalDeleted(unsigned int id)
{
	assert(id <= m_Datas.size());

	m_Datas[id] = nullptr;
	m_DeletedIndices.push_back(id);
}
