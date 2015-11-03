// copyright 2015 inbetweengames GBR

#pragma once

#include "Components/ActorComponent.h"
#include "FlockingDataCache.generated.h"

#define TEAM_CALVES 0
#define TEAM_HUNTERS 1
#define TEAM_CALVES_DEAD 2
#define TEAM_HUNTERS_DEAD 3
#define TEAM_AMBIENT 4
#define TEAM_CALVES_UNACTIVATED 5
#define TEAM_HUNTERS_PANIC 6

USTRUCT()
struct FYAgentTeamData
{
	GENERATED_BODY()

	FYAgentTeamData(int32 team) :
		m_forTeam(team),
		m_agentRadius(0.0f)
	{
	}
	FYAgentTeamData() :
		m_forTeam(0),
		m_agentRadius(0.0f)
	{
	}

	int32 m_forTeam;

	UPROPERTY(Transient)
	TArray<class ABasicAIAgent *> m_agents;

	UPROPERTY(Transient)
	TArray<class UAgentBehaviourComponent*> m_behaviours;
	
	TArray<TArray<uint8>> m_agentScratchData;

	TArray<FVector> m_locations;
	TArray<FVector> m_velocities;

	TArray<FBox> m_goalBoxes;

	TArray<FRotator> m_newFacingDirections;

	float m_agentRadius;
		
	void RemoveAgent(int32 agentIdx);
	
	UAgentBehaviourComponent *GetBehaviourOfType(UClass *ofClass) const;
};

struct YTeamChangeRequest
{
	int32 m_oldTeam;
	int32 m_newTeam;

	TWeakObjectPtr<ABasicAIAgent> m_agent;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UFlockingDataCache : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UFlockingDataCache();

	virtual void BeginPlay() override;

	// Called every frame
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;
	
	static UFlockingDataCache * GetCacheChecked(UObject *worldContextObject);

	bool IsTeamAtMaxSize(int32 teamID)
	{
		FYAgentTeamData &teamData = GetOrCreateTeamData(teamID);
		return teamData.m_agents.Num() >= m_maxTeamSize;
	}

	UFUNCTION(BlueprintCallable, Category=Teams)
	static int32 GetNumAgentsInTeam(UObject *worldContextObj, int32 team);

	FYAgentTeamData &GetOrCreateTeamData(int32 forTeam)
	{
		for (FYAgentTeamData & data : m_teamData)
		{
			if (data.m_forTeam == forTeam)
			{
				return data;
			}
		}
		return m_teamData[m_teamData.Add(FYAgentTeamData(forTeam))];
	}

	const FYAgentTeamData *GetTeamData(int32 forTeam) const
	{
		for (const FYAgentTeamData & data : m_teamData)
		{
			if (data.m_forTeam == forTeam)
			{
				return &data;
			}
		}
		return nullptr;
	}
	const FYAgentTeamData *GetTeamDataByIndex(int32 forTeamIndex) const
	{
		if (!m_teamData.IsValidIndex(forTeamIndex))
		{
			return nullptr;
		}
		return &(m_teamData[forTeamIndex]);
	}
	
	int32 GetNumTeams()
	{
		return m_teamData.Num();
	}

	const TArray<FVector>& GetLocationsAI(int32 forTeam) const
	{
		return GetTeamData(forTeam)->m_locations;
	}
	const TArray<FVector>& GetVelocitiesAI(int32 forTeam) const
	{
		return GetTeamData(forTeam)->m_velocities;
	}
	const TArray<FVector>& GetLocationsPlayer() const
	{
		return m_locationsPlayer;
	}
	const TArray<FVector>& GetVelocitiesPlayer() const
	{
		return m_velocitiesPlayer;
	}
	const TArray<FBox>& GetGoalBoxes(int32 forTeam) const
	{
		return GetTeamData(forTeam)->m_goalBoxes;
	}

	void AddGoalVolume( class AAgentGoalVolume* volume );
	void AddAIAgent(class ABasicAIAgent * agent);

	void QueueTeamChange(class ABasicAIAgent * agent, int32 newTeam);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Team)
	int32 m_maxTeamSize;

private:

	void ChangeTeam(int32 oldTeam, int32 agentIndex, int32 newTeam);

	void SetupScratchData(FYAgentTeamData & team);
	void GatherPlayerData(float deltaTime);

	void UpdateAgents(float deltaTime, int32 forTeam);

	UPROPERTY(Transient)
	TArray<FYAgentTeamData> m_teamData;

	TArray<FVector> m_locationsPlayer;
	TArray<FVector> m_velocitiesPlayer;
	
	bool m_isThreadProcessing;
	
	FGraphEventRef m_taskRef;

	TArray<YTeamChangeRequest> m_queuedTeamChanges;
};
