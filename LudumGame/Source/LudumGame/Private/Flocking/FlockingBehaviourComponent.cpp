// copyright 2015 inbetweengames GBR

#include "LudumGame.h"
#include "FlockingBehaviourComponent.h"
#include "FlockingGameMode.h"
#include "../FlockingDataCache.h"

DECLARE_CYCLE_STAT(TEXT("Flocking Behaviour"),STAT_AI_FlockingBehaviour,STATGROUP_Flocking);

// Sets default values for this component's properties
UFlockingBehaviourComponent::UFlockingBehaviourComponent()
{
	m_behaviourWeight = 10.0f;

	m_minDistance = 10.0f;

	m_avoidSameTeamOnly = false;
}

FVector UFlockingBehaviourComponent::CalcAccelerationVector(int32 thisAIIndex, const FYAgentTeamData &forTeam, class UFlockingDataCache * dataCache, uint8 *scratchData, float tickTime) const
{
	SCOPE_CYCLE_COUNTER(STAT_AI_FlockingBehaviour);

	FVector movementVector = FVector::ZeroVector;
	
	float currDistanceToClosestAI = FLT_MAX;
	FVector closestOtherLocation;
	int32 closestOtherIndex = INDEX_NONE;

	check(dataCache);

	const FVector &currentPosition = forTeam.m_locations[thisAIIndex];
	float thisAgentRadius = forTeam.m_agentRadius;

	for (int32 team = 0; team < dataCache->GetNumTeams(); team++)
	{
		const FYAgentTeamData * otherTeamData = dataCache->GetTeamDataByIndex(team);
		if (otherTeamData == nullptr)
		{
			continue;
		}

		if (m_avoidSameTeamOnly && otherTeamData->m_forTeam != forTeam.m_forTeam)
		{
			continue;
		}

		float thisTeamRadius = otherTeamData->m_agentRadius;
		float minAllowedDistance = thisAgentRadius + thisTeamRadius + m_minDistance;

		const TArray<FVector> &aiLocations = otherTeamData->m_locations;

		for (int32 i = 0; i < aiLocations.Num(); i++)
		{
			if (i == thisAIIndex && forTeam.m_forTeam == otherTeamData->m_forTeam)
			{
				continue;
			}

			FVector vectorToOtherAI = aiLocations[i] - currentPosition;
			float distanceToOtherAI = vectorToOtherAI.Size();

			if (distanceToOtherAI <= currDistanceToClosestAI && distanceToOtherAI < minAllowedDistance)
			{
				currDistanceToClosestAI = distanceToOtherAI;
				closestOtherIndex = i;
				closestOtherLocation = aiLocations[i];
			}
		}
	}

	if (closestOtherIndex != INDEX_NONE)
	{
		if (currDistanceToClosestAI <= KINDA_SMALL_NUMBER)
		{
			// basically on top: move a random unit vector
			movementVector = FMath::VRand();
			movementVector.Z = 0.0f;
		}
		else
		{
			movementVector = currentPosition - closestOtherLocation;
		}
	}

	movementVector.Normalize();

	return movementVector * m_behaviourWeight;

}
