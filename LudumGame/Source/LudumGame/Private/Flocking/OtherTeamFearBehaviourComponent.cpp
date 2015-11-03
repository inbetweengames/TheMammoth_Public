// copyright 2015 inbetweengames GBR

#include "LudumGame.h"
#include "OtherTeamFearBehaviourComponent.h"
#include "FlockingGameMode.h"
#include "../FlockingDataCache.h"

DECLARE_CYCLE_STAT(TEXT("Fear Behaviour"),STAT_AI_FearBehaviour,STATGROUP_Flocking);

// Sets default values for this component's properties
UOtherTeamFearBehaviourComponent::UOtherTeamFearBehaviourComponent()
{
	m_behaviourWeight = 2.0f;

	m_fearRadius = 400.0f;
}

// Called every frame
FVector UOtherTeamFearBehaviourComponent::CalcAccelerationVector(int32 thisAIIndex, const FYAgentTeamData &forTeam, class UFlockingDataCache * dataCache, uint8 *scratchData, float tickTime) const
{	
	SCOPE_CYCLE_COUNTER(STAT_AI_FearBehaviour);

	const FVector &thisLocation = forTeam.m_locations[thisAIIndex];

	float radiusSq = FMath::Square(m_fearRadius);

	const FYAgentTeamData *targetTeamData = dataCache->GetTeamData(m_targetTeam);
	if (targetTeamData == nullptr)
	{
		return FVector::ZeroVector;
	}

	const TArray<FVector> &otherTeamLocations = targetTeamData->m_locations;

	FVector locationSum = FVector::ZeroVector;
	int32 numNeighbours = 0;
	for (int32 i = 0; i < otherTeamLocations.Num(); i++)
	{
		if (FVector::DistSquared(otherTeamLocations[i], thisLocation) <= radiusSq)
		{
			locationSum += otherTeamLocations[i];
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
