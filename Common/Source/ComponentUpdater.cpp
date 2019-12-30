#include "ComponentUpdater.h"

void ComponentUpdater::Update(unsigned long long delta)
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

void ComponentUpdater::SignalDeleted(unsigned int id)
{
	assert(id <= m_Datas.size());

	m_Datas[id] = nullptr;
	m_DeletedIndices.push_back(id);
}
