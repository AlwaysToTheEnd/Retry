#pragma once
#include "PxPhysicsAPI.h"
#include <assert.h>
using namespace physx;

class SceneSimulationEventCallBack :public PxSimulationEventCallback
{
public:
	SceneSimulationEventCallBack();
	virtual ~SceneSimulationEventCallBack();

	virtual void onConstraintBreak(PxConstraintInfo* constraints, PxU32 count) override;
	virtual void onWake(PxActor** actors, PxU32 count) override;
	virtual void onSleep(PxActor** actors, PxU32 count) override;
	virtual void onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs) override;
	virtual void onTrigger(PxTriggerPair* pairs, PxU32 count) override;
	virtual void onAdvance(const PxRigidBody*const* bodyBuffer, const PxTransform* poseBuffer, const PxU32 count) override;

private:

};

