// copyright 2015 inbetweengames GBR

#include "LudumGame.h"
#include "FearBehaviourComponent.h"
#include "FlockingGameMode.h"
#include "../FlockingDataCache.h"

DECLARE_CYCLE_STAT(TEXT("Fear Behaviour"),STAT_AI_FearBehaviour,STATGROUP_Flocking);

// Sets default values for this component's properties
UFearBehaviourComponent::UFearBehaviourComponent()
{
	m_behaviourWeight = 2.0f;

	m_fearRadius = 400.0f;

	m_fearRadiusDeviation = 1.25f;
}

// Called every frame
FVector UFearBehaviourComponent::CalcAccelerationVector(int32 thisAIIndex, const FYAgentTeamData &forTeam, class UFlockingDataCache * dataCache, uint8 *scratchData, float tickTime) const
{	
	SCOPE_CYCLE_COUNTER(STAT_AI_FearBehaviour);

	FearAgentDeviation *deviationData = (FearAgentDeviation*)scratchData;
	if (deviationData->m_deviation <= 0.0f)
	{
		deviationData->m_deviation = FMath::FRandRange(1.0f, m_fearRadiusDeviation);
	}

	const FVector &thisLocation = forTeam.m_locations[thisAIIndex];

	float radiusSq = FMath::Square(m_fearRadius * deviationData->m_deviation);

	const TArray<FVector> &playerLocations = dataCache->GetLocationsPlayer();

	FVector locationSum = FVector::ZeroVector;
	int32 numNeighbours = 0;
	for (int32 i = 0; i < playerLocations.Num(); i++)
	{
		if (FVector::DistSquared(playerLocations[i], thisLocation) <= radiusSq)
		{
			locationSum += playerLocations[i];
			numNeighbours++;
		}
	}

	if (numNeighbours == 0)
	{
		return FVector::ZeroVector;
	}

	FVector FearLocation = locationSum / numNeighbours;
	FVector FearDirection = thisLocation - FearLocation;
	FearDirection.Normalize();
	return FearDirection * m_behaviourWeight;
}
