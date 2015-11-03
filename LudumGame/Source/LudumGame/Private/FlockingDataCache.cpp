// copyright 2015 inbetweengames GBR

#include "LudumGame.h"
#include "FlockingDataCache.h"
#include "FlockingGameMode.h"
#include "BasicAIAgent.h"
#include "AgentGoalVolume.h"
#include "DrawDebugHelpers.h"
#include "Flocking/AgentBehaviourComponent.h"
#include "Async.h"
#include "Kismet/KismetSystemLibrary.h"

DECLARE_CYCLE_STAT(TEXT("Tick Cache"), STAT_AI_CacheUpdate, STATGROUP_Flocking);
DECLARE_CYCLE_STAT(TEXT("  Cache Player Info"), STAT_CachePlayerInfo, STATGROUP_Flocking);
DECLARE_CYCLE_STAT(TEXT("  Remove Dead Agents"), STAT_RemoveDeadAgents, STATGROUP_Flocking);
DECLARE_CYCLE_STAT(TEXT("  Calculate Locations"), STAT_CalcLocations, STATGROUP_Flocking);
DECLARE_CYCLE_STAT(TEXT("  Set Agent Values"), STAT_SetLocations, STATGROUP_Flocking);
DECLARE_CYCLE_STAT(TEXT("  Process Behaviours"), STAT_ProcBehs, STATGROUP_Flocking);
DECLARE_CYCLE_STAT(TEXT("  Wait for Behaviours"), STAT_WaitBehs, STATGROUP_Flocking);
DECLARE_CYCLE_STAT(TEXT("  Calculate Velocities"), STAT_CalcVels, STATGROUP_Flocking);
DECLARE_CYCLE_STAT(TEXT("  Update Agents"), STAT_UpdateAgentsTask, STATGROUP_Flocking);
DECLARE_CYCLE_STAT(TEXT("  Update Agents Wait"), STAT_UpdateAgentsWait, STATGROUP_Flocking);

#define USE_FUTURES_FOR_BEHAVIORS 0

class FUpdateLocationsTask
{
	FYAgentTeamData&	m_teamData;
	int32 m_start;
	int32 m_end;

public:
	FUpdateLocationsTask(FYAgentTeamData &teamData, int32 StartIndex, int32 NumObjects)
		: m_teamData(teamData),
		m_start(StartIndex),
		m_end(StartIndex + NumObjects)
	{
	}
	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FUpdateLocationsTask, STATGROUP_TaskGraphTasks);
	}
	static ENamedThreads::Type GetDesiredThread()
	{
		return ENamedThreads::AnyThread;
	}
	static ESubsequentsMode::Type GetSubsequentsMode() 
	{ 
		return ESubsequentsMode::TrackSubsequents; 
	}
	void DoTask(ENamedThreads::Type CurrentThread, FGraphEventRef& MyCompletionGraphEvent)
	{
		for (int32 i = m_start; i < m_end; i++)
		{
			m_teamData.m_agents[i]->SetActorLocation(m_teamData.m_locations[i], true);
		}
		for (int32 i = m_start; i < m_end; i++)
		{
			m_teamData.m_agents[i]->SetVelocity(m_teamData.m_velocities[i], m_teamData.m_newFacingDirections[i]);
		}
	}
};


// Sets default values for this component's properties
UFlockingDataCache::UFlockingDataCache()
{
	PrimaryComponentTick.bCanEverTick = true;
	m_maxTeamSize = 50;
}

void UFlockingDataCache::BeginPlay()
{
	Super::BeginPlay();
	
	TArray<UAgentBehaviourComponent*> allBehs;
	GetOwner()->GetComponents<UAgentBehaviourComponent>(allBehs);
	for (UAgentBehaviourComponent* beh : allBehs)
	{
		GetOrCreateTeamData(beh->m_forTeam).m_behaviours.Add(beh);
	}
}

#define TEAM_CALVES 0
#define TEAM_HUNTERS 1
#define TEAM_CALVES_DEAD 2
#define TEAM_HUNTERS_DEAD 3
#define TEAM_AMBIENT 4
#define TEAM_CALVES_UNACTIVATED 5
#define TEAM_HUNTERS_PANIC 6
FString GetTeamName(int32 teamID)
{
	switch (teamID)
	{
	case TEAM_CALVES: return TEXT("calves");
	case TEAM_HUNTERS: return TEXT("hunters");
	case TEAM_CALVES_DEAD: return TEXT("deadCalves");
	case TEAM_HUNTERS_DEAD: return TEXT("deadHunters");
	case TEAM_AMBIENT: return TEXT("ambient");
	case TEAM_CALVES_UNACTIVATED: return TEXT("unactiveCalves");
	case TEAM_HUNTERS_PANIC: return TEXT("panicHunters");
	}

	return TEXT("?team?");
}

// Called every frame
void UFlockingDataCache::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	SCOPE_CYCLE_COUNTER(STAT_AI_CacheUpdate);

	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );
	
	GatherPlayerData(DeltaTime);

	for (FYAgentTeamData & team : m_teamData)
	{
		UpdateAgents(DeltaTime, team.m_forTeam);

#if !UE_BUILD_SHIPPING
		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Team %s agents %d"), *GetTeamName(team.m_forTeam), team.m_agents.Num()), true, false, FColor::White, DeltaTime);
#endif
	}	

	for (const YTeamChangeRequest & teamChange : m_queuedTeamChanges)
	{
		const FYAgentTeamData *teamData = GetTeamData(teamChange.m_oldTeam);
		check(teamData);

		if (teamChange.m_agent.IsValid())
		{
			ChangeTeam(teamChange.m_oldTeam, teamData->m_agents.IndexOfByKey(teamChange.m_agent.Get()), teamChange.m_newTeam);
		}
	}

	m_queuedTeamChanges.Reset();
	
}

void UFlockingDataCache::SetupScratchData(FYAgentTeamData & team)
{
	for (int32 iBeh = 0; iBeh < team.m_behaviours.Num(); iBeh++)	
	{
		UAgentBehaviourComponent *beh = team.m_behaviours[iBeh];
		int32 scratchDataSize = beh->GetScratchDataSize();
		int32 totalScratchDataSize = scratchDataSize * team.m_agents.Num();

		while (team.m_agentScratchData.Num() <= iBeh)
		{
			team.m_agentScratchData.Add(TArray<uint8>());
		}

		if (scratchDataSize == 0)
		{
			team.m_agentScratchData[iBeh].Empty();
		}
		else if (totalScratchDataSize > team.m_agentScratchData[iBeh].Num())
		{
			team.m_agentScratchData[iBeh].AddZeroed(totalScratchDataSize - team.m_agentScratchData[iBeh].Num());
		}
	}

}

void UFlockingDataCache::GatherPlayerData(float deltaTime)
{
	m_locationsPlayer.Reset();
	m_velocitiesPlayer.Reset();

	{
		SCOPE_CYCLE_COUNTER(STAT_CachePlayerInfo);
		for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
		{
			AController *controller = *It;
			APawn *otherPawn = controller->GetPawn();
			if (otherPawn == nullptr)
			{
				continue;
			}

			if (controller->IsA(APlayerController::StaticClass()))
			{
				if (UMovementComponent *moveComp = otherPawn->GetMovementComponent())
				{
					m_locationsPlayer.Add(otherPawn->GetActorLocation());
					m_velocitiesPlayer.Add(moveComp->Velocity);
				}
			}
		}
	}
}

void UFlockingDataCache::UpdateAgents(float DeltaTime, int32 forTeam)
{
	FYAgentTeamData &teamData = GetOrCreateTeamData(forTeam);
	
	{
		SCOPE_CYCLE_COUNTER(STAT_RemoveDeadAgents);
		// remove all dead agents
		for (int32 i = teamData.m_agents.Num() - 1; i >= 0; i--)
		{
			if (teamData.m_agents[i] == nullptr || teamData.m_agents[i]->IsPendingKill())
			{
				teamData.RemoveAgent(i);
			}
		}
	}

	int32 numAgents = teamData.m_agents.Num();
	teamData.m_locations.Reset(numAgents);
	teamData.m_velocities.Reset(numAgents);

	{
		SCOPE_CYCLE_COUNTER(STAT_CalcLocations);
		// gather all locations and velocities
		// integration location by velocity
		for (ABasicAIAgent *agent : teamData.m_agents)
		{
			FVector location = agent->GetActorLocation();
			FVector velocity = agent->GetVelocity();
			location += velocity * DeltaTime;

			teamData.m_locations.Add(location);
			teamData.m_velocities.Add(velocity);

			agent->m_currentAgentID = teamData.m_locations.Num() - 1;

	// 		if (brainComp->m_currentAgentID == 0)
	// 		{
	// 			location.Z += 100.0f;
	// 			DrawDebugDirectionalArrow(GetWorld(), location, location + (velocity), 20.0f, FColor::Yellow, false, DeltaTime * 5.0f);
	// 		}
		}
	}

	TArray<FVector> behaviourOutputs;
#if USE_FUTURES_FOR_BEHAVIORS
	TArray<TFuture< TArray<FVector>* >> behResults;
#endif
	if (numAgents > 0)
	{
		SCOPE_CYCLE_COUNTER(STAT_ProcBehs);

		behaviourOutputs.AddZeroed(numAgents);

		int32 numBehaviours = teamData.m_behaviours.Num();

#if USE_FUTURES_FOR_BEHAVIORS
		behResults.Empty(numBehaviours);

		for (int32 iBeh = 0; iBeh < numBehaviours; iBeh++)
		{
			behResults.Add(Async<TArray<FVector>*>(EAsyncExecution::TaskGraph, [=, &teamData]()
			{
				UAgentBehaviourComponent *behavior = teamData.m_behaviours[iBeh];
				TArray<FVector> *velocitiesOut = new TArray<FVector>();
				velocitiesOut->AddUninitialized(numAgents);
				for (int32 i = 0; i < numAgents; i++)
				{
					(*velocitiesOut)[i] = behavior->CalcAccelerationVector(i, teamData, this, DeltaTime);
				}
				return velocitiesOut;
			})
				);
		}
	}

	{
		SCOPE_CYCLE_COUNTER(STAT_WaitBehs);

		for (int32 iBeh = 0; iBeh < behResults.Num(); iBeh++)
		{
			TArray<FVector> *results = behResults[iBeh].Get();
			for (int32 i = 0; i < numAgents; i++)
			{
				behaviourOutputs[i] += (*results)[i];
			}
			delete results;
		}
#else
		
		for (int32 iBeh = 0; iBeh < numBehaviours; iBeh++)
		{
			UAgentBehaviourComponent *behavior = teamData.m_behaviours[iBeh];

			int32 scratchDataSize = behavior->GetScratchDataSize();
			uint8 *scratchData = nullptr;
			if (scratchDataSize > 0)
			{
				check(teamData.m_agentScratchData[iBeh].Num() >= scratchDataSize * numAgents);
				scratchData = teamData.m_agentScratchData[iBeh].GetData();
			}
			
			for (int32 i = 0; i < numAgents; i++)
			{
				if (teamData.m_agents[i]->ShouldRunBehaviours())
				{
					behaviourOutputs[i] += behavior->CalcAccelerationVector(i, teamData, this, scratchData, DeltaTime);
				}
				else
				{
					teamData.m_velocities[i] = FVector::ZeroVector;
				}

				scratchData += scratchDataSize;
			}
		}
#endif

	}

	teamData.m_newFacingDirections.Reset(numAgents);
	teamData.m_newFacingDirections.AddUninitialized(numAgents);
	
	for (int32 i = 0; i < behaviourOutputs.Num(); i++)
	{
		SCOPE_CYCLE_COUNTER(STAT_CalcVels);
		ABasicAIAgent *agent = teamData.m_agents[i];
		FVector totalAcceleration = behaviourOutputs[i];
				
		totalAcceleration.Z = 0.0f;

		totalAcceleration.Normalize();
		totalAcceleration *= agent->m_accelerationMagntiude;

		FVector &currentVelocity = teamData.m_velocities[i];

		FVector damping = -currentVelocity * agent->m_dampingFactor;

		totalAcceleration += damping;

		currentVelocity += totalAcceleration * DeltaTime;

		float newVelMag = currentVelocity.Size();
		if (newVelMag > agent->m_maximumVelocity)
		{
			currentVelocity *= agent->m_maximumVelocity / newVelMag;
		}

		FRotator newMovementDirection = currentVelocity.Rotation();
		FRotator currentFacingDirection = agent->GetActorRotation();
		teamData.m_newFacingDirections[i] = FMath::RInterpConstantTo(currentFacingDirection, newMovementDirection, DeltaTime, agent->m_agentRotationSpeed);
	}	

#define UPDATE_LOCATIONS_THREADED 0

	if (teamData.m_newFacingDirections.Num() > 0)
	{
		SCOPE_CYCLE_COUNTER(STAT_SetLocations);
#if UPDATE_LOCATIONS_THREADED

		int32 NumChunks = FMath::Min<int32>(FTaskGraphInterface::Get().GetNumWorkerThreads(), numAgents);
		int32 NumPerChunk = numAgents / NumChunks;
		check(NumPerChunk > 0);
		FGraphEventArray ChunkTasks;
		ChunkTasks.Empty(NumChunks);
		int32 StartIndex = 0;
		for (int32 Chunk = 0; Chunk < NumChunks; Chunk++)
		{
			if (Chunk + 1 == NumChunks)
			{
				NumPerChunk = numAgents - StartIndex; // last chunk takes all remaining items
			}
			ChunkTasks.Add(TGraphTask<FUpdateLocationsTask>::CreateTask().ConstructAndDispatchWhenReady(teamData, StartIndex, NumPerChunk));
			StartIndex += NumPerChunk;
		}

		QUICK_SCOPE_CYCLE_COUNTER(STAT_UpdateAgentsWait);
		FTaskGraphInterface::Get().WaitUntilTasksComplete(ChunkTasks, ENamedThreads::GameThread_Local);

	#else

		// process results from last frame
		// update all locations: this sweeps against the world
		for (int32 i = 0; i < teamData.m_locations.Num(); i++)
		{
			ABasicAIAgent *agent = teamData.m_agents[i];
			
			agent->SetActorLocation(teamData.m_locations[i], true);

			agent->SetVelocity(teamData.m_velocities[i], teamData.m_newFacingDirections[i]);
		}
#endif
	}
}

UFlockingDataCache * UFlockingDataCache::GetCacheChecked( UObject *worldContextObject )
{
	check(worldContextObject);
	check(worldContextObject->ImplementsGetWorld());

	if (AFlockingGameMode *flockMode = Cast<AFlockingGameMode>(worldContextObject->GetWorld()->GetAuthGameMode()))
	{
		check(flockMode->m_dataCache);
		return flockMode->m_dataCache;
	}
	else
	{
		check(false);
		return nullptr;
	}
}

void UFlockingDataCache::AddGoalVolume( AAgentGoalVolume* volume )
{
	GetOrCreateTeamData(volume->m_forTeam).m_goalBoxes.Add(volume->GetComponentsBoundingBox());
}

void UFlockingDataCache::AddAIAgent(class ABasicAIAgent * agent)
{
	check(agent);
	FYAgentTeamData &teamData = GetOrCreateTeamData(agent->m_teamID);
	
	teamData.m_agents.Add(agent);

	teamData.m_agentRadius = FMath::Max(teamData.m_agentRadius, agent->CalcAgentRadius());

	SetupScratchData(teamData);
}

void UFlockingDataCache::ChangeTeam(int32 oldTeam, int32 agentIndex, int32 newTeam)
{
	if (oldTeam == newTeam)
	{
		return;
	}

	if (agentIndex == INDEX_NONE)
	{
		return;
	}

	FYAgentTeamData &oldTeamData = GetOrCreateTeamData(oldTeam);
	check(agentIndex < oldTeamData.m_agents.Num());
	ABasicAIAgent *agent = oldTeamData.m_agents[agentIndex];

	oldTeamData.RemoveAgent(agentIndex);	

	agent->m_teamID = newTeam;
	AddAIAgent(agent);
	
	agent->OnTeamChanged(newTeam);
}

void UFlockingDataCache::QueueTeamChange(class ABasicAIAgent * agent, int32 newTeam)
{
	YTeamChangeRequest request;
	request.m_oldTeam = agent->m_teamID;
	request.m_agent = agent;
	request.m_newTeam = newTeam;

	m_queuedTeamChanges.Add(request);
}

int32 UFlockingDataCache::GetNumAgentsInTeam(UObject *worldContextObj, int32 team)
{
	if (worldContextObj == nullptr)
	{
		return 0;
	}

	return GetCacheChecked(worldContextObj)->GetOrCreateTeamData(team).m_agents.Num();
}

void FYAgentTeamData::RemoveAgent(int32 agentIdx)
{
	m_agents.RemoveAt(agentIdx);

	if (agentIdx < m_locations.Num())
	{
		m_locations.RemoveAt(agentIdx);
	}
	if (agentIdx < m_velocities.Num())
	{
		m_velocities.RemoveAt(agentIdx);
	}
	if (agentIdx < m_newFacingDirections.Num())
	{
		m_newFacingDirections.RemoveAt(agentIdx);
	}

	for (int32 iBeh = 0; iBeh < m_behaviours.Num(); iBeh++)	
	{
		UAgentBehaviourComponent *beh = m_behaviours[iBeh];

		int32 scratchDataSize = beh->GetScratchDataSize();

		if (scratchDataSize > 0)
		{
			int32 offset = scratchDataSize * agentIdx;
			m_agentScratchData[iBeh].RemoveAt(offset, scratchDataSize);
		}
	}

}

UAgentBehaviourComponent * FYAgentTeamData::GetBehaviourOfType(UClass *ofClass) const
{
	for (UAgentBehaviourComponent * comp : m_behaviours)
	{
		if (comp->IsA(ofClass))
		{
			return comp;
		}
	}

	return nullptr;
}
