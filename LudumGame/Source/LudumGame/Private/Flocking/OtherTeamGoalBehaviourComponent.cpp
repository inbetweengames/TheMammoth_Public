// copyright 2015 inbetweengames GBR

#include "LudumGame.h"
#include "OtherTeamGoalBehaviourComponent.h"
#include "FlockingGameMode.h"
#include "AI/Navigation/NavigationPath.h"
#include "DrawDebugHelpers.h"
#include "../FlockingDataCache.h"
#include "BasicAIAgent.h"

DECLARE_CYCLE_STAT(TEXT("Goal Behaviour"),STAT_AI_GoalBehaviour,STATGROUP_Flocking);

// Sets default values for this component's properties
UOtherTeamGoalBehaviourComponent::UOtherTeamGoalBehaviourComponent()
{
	m_behaviourWeight = 1.0f;

	m_maxDistance = 10000.0f;
}

FVector UOtherTeamGoalBehaviourComponent::CalcAccelerationVector(int32 thisAIIndex, const FYAgentTeamData &forTeam, class UFlockingDataCache * dataCache, uint8 *scratchData, float tickTime) const
{
	SCOPE_CYCLE_COUNTER(STAT_AI_GoalBehaviour);

	const FVector &thisLocation = forTeam.m_locations[thisAIIndex];

	float closest = MAX_FLT;
	FVector closestLocation = FVector::ZeroVector;

	for (int32 otherTeam : m_targetTeams)
	{
		const FYAgentTeamData *targetTeamData = dataCache->GetTeamData(otherTeam);
		if (targetTeamData == nullptr)
		{
			continue;
		}

		const TArray<FVector> &targetLocations = targetTeamData->m_locations;
		if (targetLocations.Num() == 0)
		{
			continue;
		}

		for (const FVector & targetLocation : targetLocations)
		{
			float distSq = FVector::DistSquared(thisLocation, targetLocation);
			if (distSq < closest)
			{
				closest = distSq;
				closestLocation = targetLocation;
			}
		}

	}

	if (!closestLocation.IsNearlyZero())
	{
		FVector toLocation = closestLocation - thisLocation;
		toLocation.Z = 0.0f;

		if (toLocation.Size() <= m_maxDistance)
		{
			toLocation.Normalize();

			return toLocation * m_behaviourWeight;
		}
	}

	if (dataCache->GetLocationsPlayer().Num() > 0)
	{
		FVector player = dataCache->GetLocationsPlayer()[0];

		FVector toLocation = player - thisLocation;
		toLocation.Z = 0.0f;

		toLocation.Normalize();
		
		return toLocation * m_behaviourWeight;
	}
	
	return FVector::ZeroVector;
}
