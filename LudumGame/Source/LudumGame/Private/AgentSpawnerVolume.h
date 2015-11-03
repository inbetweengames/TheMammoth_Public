// copyright 2015 inbetweengames GBR

#pragma once

#include "GameFramework/Volume.h"
#include "AgentSpawnerVolume.generated.h"

UCLASS()
class AAgentSpawnerVolume : public AVolume
{
	GENERATED_BODY()

	AAgentSpawnerVolume(const FObjectInitializer& ObjectInitializer);
	
public:

	UFUNCTION(BlueprintCallable, Category=Spawning)
	void StartSpawning(int32 numToSpawn);
	
	UFUNCTION(BlueprintCallable, Category = Spawning)
	void StopSpawning();

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Spawning)
	TSubclassOf<class ABasicAIAgent> m_agentToSpawn;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Spawning)
	int32 m_numAgents;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Spawning)
	float m_spawnDelay;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Spawning)
	bool m_spawnerActive;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Spawning)
	bool m_endlessMode;

	virtual void Tick(float DeltaSeconds) override;

private:

	int32 m_numAgentsSpawned;

	float m_timeSinceSpawn;
};