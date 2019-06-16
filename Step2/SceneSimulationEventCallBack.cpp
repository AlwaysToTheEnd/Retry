#include "SceneSimulationEventCallBack.h"



SceneSimulationEventCallBack::SceneSimulationEventCallBack()
{
}


SceneSimulationEventCallBack::~SceneSimulationEventCallBack()
{
}

void SceneSimulationEventCallBack::onConstraintBreak(PxConstraintInfo * constraints, PxU32 count)
{
}

void SceneSimulationEventCallBack::onWake(PxActor ** actors, PxU32 count)
{
	
}

void SceneSimulationEventCallBack::onSleep(PxActor ** actors, PxU32 count)
{
}

void SceneSimulationEventCallBack::onContact(const PxContactPairHeader & pairHeader, const PxContactPair * pairs, PxU32 nbPairs)
{
}

void SceneSimulationEventCallBack::onTrigger(PxTriggerPair * pairs, PxU32 count)
{

}

void SceneSimulationEventCallBack::onAdvance(const PxRigidBody * const * bodyBuffer, const PxTransform * poseBuffer, const PxU32 count)
{
}
