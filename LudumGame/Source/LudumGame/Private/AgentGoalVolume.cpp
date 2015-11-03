// copyright 2015 inbetweengames GBR

#include "LudumGame.h"

#include "AgentGoalVolume.h"
#include "FlockingDataCache.h"
#include "BasicAIAgent.h"


AAgentGoalVolume::AAgentGoalVolume(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	m_forTeam = 0;

	m_destroyOnEntering = true;
	m_killOnEntering = false;
}

void AAgentGoalVolume::BeginPlay()
{
	Super::BeginPlay();

	UFlockingDataCache::GetCacheChecked(this)->AddGoalVolume(this);
}

void AAgentGoalVolume::NotifyActorBeginOverlap( AActor* OtherActor )
{
	Super::NotifyActorBeginOverlap(OtherActor);

	ABasicAIAgent *agent = Cast<ABasicAIAgent>(OtherActor);
	if (agent && agent->m_teamID == m_forTeam)
	{
		if (m_destroyOnEntering)
		{
			agent->Destroy();
		}
		else if (m_killOnEntering)
		{
			agent->ModifyHealth(-agent->GetHealth());
		}
	}
}
