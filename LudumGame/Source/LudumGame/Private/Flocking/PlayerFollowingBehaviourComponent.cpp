// copyright 2015 inbetweengames GBR

#include "LudumGame.h"
#include "PlayerFollowingBehaviourComponent.h"
#include "FlockingGameMode.h"
#include "../FlockingDataCache.h"
#include "CalfAgent.h"

DECLARE_CYCLE_STAT(TEXT("Fear Behaviour"),STAT_AI_FearBehaviour,STATGROUP_Flocking);

// Sets default values for this component's properties
UPlayerFollowingBehaviourComponent::UPlayerFollowingBehaviourComponent()
{
	m_behaviourWeight = 2.0f;

	m_minRadius = 400.0f;
	m_maxRadius = 10000.0f;
}

// Called every frame
FVector UPlayerFollowingBehaviourComponent::CalcAccelerationVector(int32 thisAIIndex, const FYAgentTeamData &forTeam, class UFlockingDataCache * dataCache, uint8 *scratchData, float tickTime) const
{
	SCOPE_CYCLE_COUNTER(STAT_AI_FearBehaviour);

	const FVector &thisLocation = forTeam.m_locations[thisAIIndex];

	float agentRadius = 0.0f;

	const TArray<FVector> &playerLocations = dataCache->GetLocationsPlayer();
	ACalfAgent *thisCalf = Cast<ACalfAgent>(forTeam.m_agents[thisAIIndex]);
	FVector targetPosition = FVector::ZeroVector;
	if (thisCalf)
	{
		int32 thisCalfSize = thisCalf->m_currentSize;

		agentRadius += thisCalf->m_sizes[thisCalfSize].m_capsuleRadius;

		int32 targetSize = thisCalfSize + 1;
		while (targetSize < thisCalf->m_sizes.Num())
		{
			for (int32 i = 0; i < forTeam.m_agents.Num(); i++)
			{
				ACalfAgent *otherCalf = Cast<ACalfAgent>(forTeam.m_agents[i]);
				if (otherCalf && otherCalf->m_currentSize == targetSize && otherCalf->IsAlive())
				{
					// found a target to follow
					targetPosition = forTeam.m_locations[i];

					agentRadius += thisCalf->m_sizes[targetSize].m_capsuleRadius;

					break;
				}
			}

			if (!targetPosition.IsNearlyZero())
			{
				break;
			}

			targetSize++;
		}

		if (targetPosition.IsNearlyZero())
		{
			agentRadius += 400.0f;

			targetPosition = playerLocations[0];
		}
		
	}
	else
	{
		targetPosition = playerLocations[0];
	}

	float minRadiusSq = FMath::Square(agentRadius + m_minRadius);
	float maxRadiusSq = FMath::Square(agentRadius + m_maxRadius);
	
	float distSq = FVector::DistSquared(targetPosition, thisLocation);
	if (distSq <= minRadiusSq || distSq >= maxRadiusSq)
	{
		return FVector::ZeroVector;
	}

	FVector playerDirection = targetPosition - thisLocation;
	playerDirection.Normalize();
	return playerDirection * m_behaviourWeight;
}
