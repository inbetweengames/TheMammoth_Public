// copyright 2015 inbetweengames GBR

#include "LudumGame.h"
#include "GoalBehaviourComponent.h"
#include "FlockingGameMode.h"
#include "AI/Navigation/NavigationPath.h"
#include "DrawDebugHelpers.h"
#include "../FlockingDataCache.h"

DECLARE_CYCLE_STAT(TEXT("Goal Behaviour"),STAT_AI_GoalBehaviour,STATGROUP_Flocking);

// Sets default values for this component's properties
UGoalBehaviourComponent::UGoalBehaviourComponent()
{
	m_behaviourWeight = 1.0f;
	
	m_reevaluationTime = 1.0f;
	m_reevaluationTimeRand = 0.1f;
}

FVector UGoalBehaviourComponent::CalcAccelerationVector(int32 thisAIIndex, const FYAgentTeamData &forTeam, class UFlockingDataCache * dataCache, uint8 *scratchData, float tickTime) const
{
	SCOPE_CYCLE_COUNTER(STAT_AI_GoalBehaviour);

	FGoalScratchData *thisAgentData = (FGoalScratchData*)scratchData;

	thisAgentData->m_timeToRevaluation -= tickTime;
	FVector thisLocation = forTeam.m_locations[thisAIIndex];

	if (thisAgentData->m_timeToRevaluation < 0.0f || thisAgentData->m_currentPathDest.IsNearlyZero())
	{
		const TArray<FBox> &boxes = forTeam.m_goalBoxes;
		if (boxes.Num() == 0)
		{
			return FVector::ZeroVector;
		}

		bool alreadyHasGoal = !thisAgentData->m_currentGoal.IsNearlyZero() &&
			FVector::Dist(thisAgentData->m_currentGoal, thisLocation) >= forTeam.m_agentRadius;
		if (!alreadyHasGoal)
		{
			float closest = MAX_FLT;

			for (const FBox & box : boxes)
			{
				FVector boxGoal;
				if (m_goToClosestPoint)
				{
					boxGoal = box.GetClosestPointTo(thisLocation);
				}
				else
				{
					boxGoal = FMath::RandPointInBox(box);
				}

				float distSq = FVector::DistSquared(thisLocation, boxGoal);
				if (distSq < closest)
				{
					closest = distSq;
					thisAgentData->m_currentGoal = boxGoal;
				}
			}
		}

/*		AActor *nonConstActor = (AActor*)(forTeam.m_agents[thisAIIndex]);*/

		thisLocation.Z = thisAgentData->m_currentGoal.Z;
		UNavigationPath *path = UNavigationSystem::FindPathToLocationSynchronously(GetWorld(), thisLocation, thisAgentData->m_currentGoal, nullptr);
		if (path && path->PathPoints.Num() > 1)
		{
			thisAgentData->m_currentPathDest = path->PathPoints[1];
		}

		thisAgentData->m_timeToRevaluation = m_reevaluationTime + FMath::FRandRange(-m_reevaluationTimeRand, m_reevaluationTimeRand);
		

// 		if (path && /*m_path->IsValid() &&*/ path->PathPoints.Num() > 1)
// 		{
// 			for (int i = 0; i < path->PathPoints.Num() - 1; i++)
// 			{
// 				DrawDebugLine(GetWorld(), path->PathPoints[i], path->PathPoints[i + 1], FColor::Red, false, thisAgentData->m_timeToRevaluation);
// 			}
// 		}

		//DrawDebugLine(GetWorld(), thisLocation, closestGoal, FColor::Yellow, false, thisAgentData->m_timeToRevaluation);
	}

	if (!thisAgentData->m_currentPathDest.IsNearlyZero())
	{
		FVector toFirstPoint = thisAgentData->m_currentPathDest - thisLocation;
		toFirstPoint.Z = 0.0f;

		if (toFirstPoint.Size() < (forTeam.m_agentRadius))
		{
			// reached destination
			thisAgentData->m_currentPathDest = FVector::ZeroVector;
			thisAgentData->m_currentGoal = FVector::ZeroVector;
		}
		
		toFirstPoint.Normalize();
		return toFirstPoint * m_behaviourWeight;
	}
	else
	{
		return FVector::ZeroVector;
	}
}
