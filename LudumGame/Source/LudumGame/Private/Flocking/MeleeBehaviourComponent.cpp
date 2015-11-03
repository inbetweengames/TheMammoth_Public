// copyright 2015 inbetweengames GBR

#include "LudumGame.h"
#include "MeleeBehaviourComponent.h"
#include "FlockingGameMode.h"
#include "../FlockingDataCache.h"
#include "BasicAIAgent.h"

DECLARE_CYCLE_STAT(TEXT("Flocking Behaviour"),STAT_AI_FlockingBehaviour,STATGROUP_Flocking);

// Sets default values for this component's properties
UMeleeBehaviourComponent::UMeleeBehaviourComponent()
{
	m_meleeDistance = 10.0f;
	m_damagePerSecond = 1.0f;
}

FVector UMeleeBehaviourComponent::CalcAccelerationVector(int32 thisAIIndex, const FYAgentTeamData &forTeam, class UFlockingDataCache * dataCache, uint8 *scratchData, float tickTime) const
{
	SCOPE_CYCLE_COUNTER(STAT_AI_FlockingBehaviour);

	check(dataCache);

	const FVector &currentPosition = forTeam.m_locations[thisAIIndex];
	float thisAgentRadius = forTeam.m_agentRadius;

	float damageThisTick = m_damagePerSecond * tickTime;

	for (int32 team = 0; team < dataCache->GetNumTeams(); team++)
	{
		const FYAgentTeamData * teamData = dataCache->GetTeamDataByIndex(team);
		if (teamData == nullptr)
		{
			continue;
		}

		if (teamData->m_forTeam == forTeam.m_forTeam)
		{
			continue;
		}

		float thisTeamRadius = teamData->m_agentRadius;
		float minAllowedDistance = thisAgentRadius + thisTeamRadius + m_meleeDistance;

		const TArray<FVector> &aiLocations = teamData->m_locations;

		for (int32 i = 0; i < aiLocations.Num(); i++)
		{
			FVector vectorToOtherAI = aiLocations[i] - currentPosition;
			float distanceToOtherAI = vectorToOtherAI.Size();

			if (distanceToOtherAI < minAllowedDistance)
			{
				teamData->m_agents[i]->ModifyHealth(-damageThisTick);
			}
		}
	}

	return FVector::ZeroVector;

}
