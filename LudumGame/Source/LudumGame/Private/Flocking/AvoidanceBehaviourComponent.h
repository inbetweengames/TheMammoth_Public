// copyright 2015 inbetweengames GBR

#pragma once

#include "AgentBehaviourComponent.h"
#include "AvoidanceBehaviourComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UAvoidanceBehaviourComponent : public UAgentBehaviourComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAvoidanceBehaviourComponent();

	void InitializeComponent();

	FVector CalcAccelerationVector(int32 thisAIIndex, const FYAgentTeamData &forTeam, class UFlockingDataCache * dataCache, uint8 *scratchData, float tickTime) const override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=AgentBehavior)
	float m_behaviourWeight;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category=AgentBehavior)
	float m_avoidanceRadius;

	TArray<FVector, TInlineAllocator<8>> m_radiusVectors;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category=AgentBehavior)
	float m_meshZ;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = AgentBehavior)
	float m_drawDebug;
};
