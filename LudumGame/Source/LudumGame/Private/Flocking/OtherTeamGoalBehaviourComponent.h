// copyright 2015 inbetweengames GBR

#pragma once

#include "AgentBehaviourComponent.h"
#include "OtherTeamGoalBehaviourComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UOtherTeamGoalBehaviourComponent : public UAgentBehaviourComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UOtherTeamGoalBehaviourComponent();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=AgentBehavior)
	float m_behaviourWeight;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=AgentBehavior)
	float m_maxDistance;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = AgentBehavior)
	TArray<int32> m_targetTeams;

	FVector CalcAccelerationVector(int32 thisAIIndex, const FYAgentTeamData &forTeam, class UFlockingDataCache * dataCache, uint8 * scratchData, float tickTime) const override;

private:

};
