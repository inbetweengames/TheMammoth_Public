// copyright 2015 inbetweengames GBR

#include "LudumGame.h"
#include "AvoidanceBehaviourComponent.h"
#include "FlockingGameMode.h"
#include "DrawDebugHelpers.h"
#include "../FlockingDataCache.h"
#include "AI/Navigation/NavigationPath.h"

DECLARE_CYCLE_STAT(TEXT("Avoidance Behaviour"),STAT_AI_AvoidanceBehaviour,STATGROUP_Flocking);

// Sets default values for this component's properties
UAvoidanceBehaviourComponent::UAvoidanceBehaviourComponent()
{
	m_behaviourWeight = 1.0f;

	m_avoidanceRadius = 80.0f;

	bWantsInitializeComponent = true;

	m_meshZ = 365.0f;

	m_drawDebug = false;
}

void UAvoidanceBehaviourComponent::InitializeComponent()
{
	Super::InitializeComponent();

	float diagonalVectorXY = FMath::Cos(PI * 0.25) * m_avoidanceRadius;

	m_radiusVectors.Add(FVector(m_avoidanceRadius, 0.0f, 0.0f));
	m_radiusVectors.Add(FVector(-m_avoidanceRadius, 0.0f, 0.0f));
	m_radiusVectors.Add(FVector(0.0f, m_avoidanceRadius, 0.0f));
	m_radiusVectors.Add(FVector(0.0f, -m_avoidanceRadius, 0.0f));
	m_radiusVectors.Add(FVector(diagonalVectorXY, diagonalVectorXY, 0.0f));
	m_radiusVectors.Add(FVector(-diagonalVectorXY, diagonalVectorXY, 0.0f));
	m_radiusVectors.Add(FVector(-diagonalVectorXY, -diagonalVectorXY, 0.0f));
	m_radiusVectors.Add(FVector(diagonalVectorXY, -diagonalVectorXY, 0.0f));
}

FVector UAvoidanceBehaviourComponent::CalcAccelerationVector(int32 thisAIIndex, const FYAgentTeamData &forTeam, class UFlockingDataCache * dataCache, uint8 *scratchData, float tickTime) const
{
	SCOPE_CYCLE_COUNTER(STAT_AI_AvoidanceBehaviour);

	FVector thisLocation = forTeam.m_locations[thisAIIndex];

	thisLocation.Z = m_meshZ;

	FVector hitLocation;
	for (const FVector &radiusVector : m_radiusVectors)
	{
		FVector lineTraceEnd = thisLocation + radiusVector;

		//UNavigationPath *foundPath = UNavigationSystem::FindPathToLocationSynchronously(this, thisLocation, lineTraceEnd);

		bool obstructed = /*foundPath == nullptr || !foundPath->IsValid() || foundPath->PathPoints.Num() == 0;*/
			UNavigationSystem::NavigationRaycast(GetWorld(), thisLocation, lineTraceEnd, hitLocation, nullptr, nullptr);

		if (obstructed)
		{
			// avoid the thing in front of us
			FVector avoidanceDirection = -radiusVector;
			avoidanceDirection.Normalize();
			
			if (m_drawDebug)
			{
				DrawDebugLine(GetWorld(), thisLocation, hitLocation, FColor::Red, false, 1.0f);
			}

			return avoidanceDirection * m_behaviourWeight;
		}
	}

	return FVector::ZeroVector;
}
