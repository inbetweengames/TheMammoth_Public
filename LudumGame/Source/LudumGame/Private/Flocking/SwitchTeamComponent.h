// copyright 2015 inbetweengames GBR

#pragma once

#include "AgentBehaviourComponent.h"
#include "SwitchTeamComponent.generated.h"

USTRUCT()
struct FSwitchTeamScratchData
{
	GENERATED_BODY()

	FSwitchTeamScratchData() :
		m_timeUntilChange(0.0f)
	{

	}

	float m_timeUntilChange;

};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class USwitchTeamComponent : public UAgentBehaviourComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USwitchTeamComponent();

	FVector CalcAccelerationVector(int32 thisAIIndex, const FYAgentTeamData &forTeam, class UFlockingDataCache * dataCache, uint8 * scratchData, float tickTime) const override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=AgentBehavior)
	float m_minTimeTillTeamChange;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=AgentBehavior)
	float m_maxTimeTillTeamChange;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=AgentBehavior)
	TArray<int32> m_possibleTeams;
	
	virtual int32 GetScratchDataSize() { return sizeof(FSwitchTeamScratchData); }

private:

};
