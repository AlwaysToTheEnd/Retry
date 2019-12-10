#include "ComponentUpdater.h"

void ComponentUpdater::Update()
{
	for (auto& it : m_Datas)
	{
		if (it != nullptr)
		{
			if (it->IsActive())
			{
				it->Update();
			}
		}
	}
}

void ComponentUpdater::SignalDelete(unsigned int id)
{
	assert(id <= m_Datas.size());

	m_Datas[id] = nullptr;
	m_DeletedIndices.push_back(id);
}
