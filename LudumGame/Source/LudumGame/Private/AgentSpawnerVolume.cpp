// copyright 2015 inbetweengames GBR

#include "LudumGame.h"

#include "AgentSpawnerVolume.h"
#include "FlockingDataCache.h"
#include "BasicAIAgent.h"


AAgentSpawnerVolume::AAgentSpawnerVolume(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	m_numAgents = 1;
	m_spawnDelay = 0.01f;

	PrimaryActorTick.bCanEverTick = true;

	m_numAgentsSpawned = 0;
	m_timeSinceSpawn = 0;

	m_spawnerActive = false;
	m_endlessMode = false;
}

void AAgentSpawnerVolume::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (m_spawnerActive == false)
	{
		return;
	}

	int32 numToSpawn = 0;
	m_timeSinceSpawn += DeltaSeconds;
	while (m_timeSinceSpawn >= m_spawnDelay)
	{
		numToSpawn++;
		m_timeSinceSpawn -= m_spawnDelay;
	}

	if (!m_endlessMode)
	{
		numToSpawn = FMath::Min(numToSpawn, m_numAgents - m_numAgentsSpawned);
	}
	FBox box = GetComponentsBoundingBox();

	ABasicAIAgent *agent = Cast<ABasicAIAgent>(m_agentToSpawn.GetDefaultObject());
	int32 agentTeam = agent->m_teamID;

	UFlockingDataCache *dataCache = UFlockingDataCache::GetCacheChecked(this);

	for (int32 i = 0; i < numToSpawn; i++)
	{
		if (!dataCache->IsTeamAtMaxSize(agentTeam))
		{
			FVector point = FMath::RandPointInBox(FBox(box.Min, box.Max));
			point.Z += agent->m_capsuleComponent->GetScaledCapsuleHalfHeight();

			GetWorld()->SpawnActor(*m_agentToSpawn, &point, &FRotator::ZeroRotator, FActorSpawnParameters());
		}

		m_numAgentsSpawned++;
	}
}

void AAgentSpawnerVolume::StartSpawning(int32 numToSpawn)
{
	m_spawnerActive = true;
	m_numAgentsSpawned = 0;
	m_numAgents = numToSpawn;
}

void AAgentSpawnerVolume::StopSpawning()
{
	m_spawnerActive = false;
}
