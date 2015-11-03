// copyright 2015 inbetweengames GBR

#pragma once

#include "AgentBehaviourComponent.h"
#include "FearBehaviourComponent.generated.h"

struct FearAgentDeviation
{
	FearAgentDeviation() :
		m_deviation(-1.0f)
	{

	}

	float m_deviation;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UFearBehaviourComponent : public UAgentBehaviourComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UFearBehaviourComponent();


	FVector CalcAccelerationVector(int32 thisAIIndex, const FYAgentTeamData &forTeam, class UFlockingDataCache * dataCache, uint8 *scratchData, float tickTime) const;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=AgentBehavior)
	float m_behaviourWeight;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=AgentBehavior)
	float m_fearRadius;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=AgentBehavior)
	float m_fearRadiusDeviation;

	int32 GetScratchDataSize() override { return sizeof(FearAgentDeviation); }
};
