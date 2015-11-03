// copyright 2015 inbetweengames GBR

#pragma once

#include "AgentBehaviourComponent.h"
#include "PlayerFollowingBehaviourComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UPlayerFollowingBehaviourComponent : public UAgentBehaviourComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPlayerFollowingBehaviourComponent();


	FVector CalcAccelerationVector(int32 thisAIIndex, const FYAgentTeamData &forTeam, class UFlockingDataCache * dataCache, uint8 *scratchData, float tickTime) const;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=AgentBehavior)
	float m_behaviourWeight;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=AgentBehavior)
	float m_minRadius;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=AgentBehavior)
	float m_maxRadius;
};
