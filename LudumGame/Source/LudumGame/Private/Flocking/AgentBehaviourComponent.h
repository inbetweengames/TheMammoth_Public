// copyright 2015 inbetweengames GBR

#pragma once

#include "Components/ActorComponent.h"
#include "../FlockingDataCache.h"
#include "AgentBehaviourComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UAgentBehaviourComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAgentBehaviourComponent();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = AgentBehavior)
	int32 m_forTeam;
			
	virtual FVector CalcAccelerationVector(int32 thisAIIndex, const FYAgentTeamData &forTeam, class UFlockingDataCache * dataCache, uint8 *scratchData, float tickTime) const;
	
	virtual int32 GetScratchDataSize() { return 0; }
};
