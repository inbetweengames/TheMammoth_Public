// copyright 2015 inbetweengames GBR

#pragma once

#include "AgentBehaviourComponent.h"
#include "MeleeBehaviourComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UMeleeBehaviourComponent : public UAgentBehaviourComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UMeleeBehaviourComponent();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=AgentBehavior)
	float m_meleeDistance;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = AgentBehavior)
	float m_damagePerSecond;

	FVector CalcAccelerationVector(int32 thisAIIndex, const FYAgentTeamData &forTeam, class UFlockingDataCache * dataCache, uint8 *scratchData, float tickTime) const override;

};
