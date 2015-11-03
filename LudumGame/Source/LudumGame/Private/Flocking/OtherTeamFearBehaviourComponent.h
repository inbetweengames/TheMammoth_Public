// copyright 2015 inbetweengames GBR

#pragma once

#include "AgentBehaviourComponent.h"
#include "OtherTeamFearBehaviourComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UOtherTeamFearBehaviourComponent : public UAgentBehaviourComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UOtherTeamFearBehaviourComponent();


	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = AgentBehavior)
	int32 m_targetTeam;

	FVector CalcAccelerationVector(int32 thisAIIndex, const FYAgentTeamData &forTeam, class UFlockingDataCache * dataCache, uint8 *scratchData, float tickTime) const;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=AgentBehavior)
	float m_behaviourWeight;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=AgentBehavior)
	float m_fearRadius;
};
