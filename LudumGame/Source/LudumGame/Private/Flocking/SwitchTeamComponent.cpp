// copyright 2015 inbetweengames GBR

#include "LudumGame.h"
#include "SwitchTeamComponent.h"
#include "FlockingGameMode.h"
#include "AI/Navigation/NavigationPath.h"
#include "DrawDebugHelpers.h"
#include "../FlockingDataCache.h"

DECLARE_CYCLE_STAT(TEXT("Goal Behaviour"),STAT_AI_GoalBehaviour,STATGROUP_Flocking);

// Sets default values for this component's properties
USwitchTeamComponent::USwitchTeamComponent()
{
	m_minTimeTillTeamChange = 10.0f;
	m_maxTimeTillTeamChange = 60.0f;
}

FVector USwitchTeamComponent::CalcAccelerationVector(int32 thisAIIndex, const FYAgentTeamData &forTeam, class UFlockingDataCache * dataCache, uint8 *scratchData, float tickTime) const
{
	SCOPE_CYCLE_COUNTER(STAT_AI_GoalBehaviour);

	FSwitchTeamScratchData *thisAgentData = (FSwitchTeamScratchData*)scratchData;

	if (thisAgentData->m_timeUntilChange <= 0.0f)
	{
		thisAgentData->m_timeUntilChange = FMath::FRandRange(m_minTimeTillTeamChange, m_maxTimeTillTeamChange);
	}
	else
	{
		thisAgentData->m_timeUntilChange -= tickTime;

		//DrawDebugString(GetWorld(), forTeam.m_locations[thisAIIndex], FString::Printf(TEXT("%.1f"), thisAgentData->m_timeUntilChange), nullptr, FColor::Red, tickTime*2.0f);

		if (thisAgentData->m_timeUntilChange <= 0.0f && m_possibleTeams.Num() > 0)
		{
			int32 newTeamIdx = FMath::RandRange(0, m_possibleTeams.Num()-1);
			int32 newTeam = m_possibleTeams[newTeamIdx];

			dataCache->QueueTeamChange(forTeam.m_agents[thisAIIndex], newTeam);
		}
	}

	return FVector::ZeroVector;
}
