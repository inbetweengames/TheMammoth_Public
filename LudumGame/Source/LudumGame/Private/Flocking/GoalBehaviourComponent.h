// copyright 2015 inbetweengames GBR

#pragma once

#include "AgentBehaviourComponent.h"
#include "GoalBehaviourComponent.generated.h"

USTRUCT()
struct FGoalScratchData
{
	GENERATED_BODY()

	FGoalScratchData() :
		m_timeToRevaluation(0.0f)
	{

	}
	
	float m_timeToRevaluation;

	FVector m_currentGoal;

	FVector m_currentPathDest;
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UGoalBehaviourComponent : public UAgentBehaviourComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGoalBehaviourComponent();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=AgentBehavior)
	float m_behaviourWeight;

	FVector CalcAccelerationVector(int32 thisAIIndex, const FYAgentTeamData &forTeam, class UFlockingDataCache * dataCache, uint8 * scratchData, float tickTime) const override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=AgentBehavior)
	float m_reevaluationTime;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=AgentBehavior)
	float m_reevaluationTimeRand;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Goal)
	bool m_goToClosestPoint;
	
	virtual int32 GetScratchDataSize() { return sizeof(FGoalScratchData); }

private:

};
