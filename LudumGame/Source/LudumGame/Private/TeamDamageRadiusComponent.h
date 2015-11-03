// copyright 2015 inbetweengames GBR

#pragma once

#include "TeamDamageRadiusComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UTeamDamageRadiusComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTeamDamageRadiusComponent();

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Damage)
	float m_damageRadius;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Damage)
	float m_damagePerSecond;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Damage)
	TArray<int32> m_teamsToDamage;

};
