#include "IComponentCreater.h"

ComponentUpdater ICompnentCreater::m_ComUpdater[IComponent::NUMCOMPONENTTYPE];

ICompnentCreater::ICompnentCreater()
{

}

ComponentUpdater& ICompnentCreater::GetComponentUpdater(COMPONENTTYPE type)
{
	unsigned int index = static_cast<unsigned int>(type);

	return m_ComUpdater[index];
}
