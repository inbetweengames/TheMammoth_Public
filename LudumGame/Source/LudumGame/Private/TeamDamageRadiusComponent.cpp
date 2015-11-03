// copyright 2015 inbetweengames GBR

#include "LudumGame.h"
#include "TeamDamageRadiusComponent.h"
#include "FlockingDataCache.h"
#include "BasicAIAgent.h"


// Sets default values for this component's properties
UTeamDamageRadiusComponent::UTeamDamageRadiusComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	bAutoActivate = true;
}

void UTeamDamageRadiusComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UFlockingDataCache *dataCache = UFlockingDataCache::GetCacheChecked(this);
		
	FVector currentPosition = GetOwner()->GetActorLocation();

	/*UCapsuleComponent *capsule = GetOwner()->FindComponentByClass<UCapsuleComponent>();
	if (capsule)
	{
		currentPosition.Z -= capsule->GetScaledCapsuleHalfHeight();
	}*/

	float damageThisTick = m_damagePerSecond * DeltaTime;

	for (int32 team : m_teamsToDamage)
	{
		const FYAgentTeamData *teamData = dataCache->GetTeamData(team);

		float thisTeamRadius = teamData->m_agentRadius;

		const TArray<FVector> &aiLocations = teamData->m_locations;

		for (int32 i = 0; i < aiLocations.Num(); i++)
		{
			FVector vectorToOtherAI = aiLocations[i] - currentPosition;
			float distanceToOtherAI = vectorToOtherAI.Size();

			if (distanceToOtherAI < m_damageRadius)
			{
				teamData->m_agents[i]->ModifyHealth(-damageThisTick);
			}
		}
	}
}
