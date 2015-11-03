// copyright 2015 inbetweengames GBR

#include "LudumGame.h"
#include "AgentBehaviourComponent.h"
#include "FlockingGameMode.h"

DECLARE_CYCLE_STAT(TEXT("Cache Update"),STAT_AI_CacheUpdate,STATGROUP_Flocking);

// Sets default values for this component's properties
UAgentBehaviourComponent::UAgentBehaviourComponent()
{
	m_forTeam = 0;
}

FVector UAgentBehaviourComponent::CalcAccelerationVector(int32 thisAIIndex, const FYAgentTeamData &forTeam, class UFlockingDataCache * dataCache, uint8 *scratchData, float tickTime) const
{
	return FVector::ZeroVector;
}
