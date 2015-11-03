// copyright 2015 inbetweengames GBR

#pragma once

#include "GameFramework/Volume.h"
#include "AgentGoalVolume.generated.h"

UCLASS()
class AAgentGoalVolume : public AVolume
{
	GENERATED_BODY()

	AAgentGoalVolume(const FObjectInitializer& ObjectInitializer);

	void BeginPlay() override;

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

public:

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category=Goal)
	int32 m_forTeam;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Goal)
	bool m_destroyOnEntering;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Goal)
	bool m_killOnEntering;
};