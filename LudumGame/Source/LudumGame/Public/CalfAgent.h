// copyright 2015 inbetweengames GBR

#pragma once

#include "BasicAIAgent.h"
#include "CalfAgent.generated.h"

USTRUCT()
struct FCalfSizeSetup
{
	GENERATED_BODY()

	FCalfSizeSetup()
	{
		m_health = 100.0f;
	}

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Size)
	float m_scale;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Size)
	float m_capsuleRadius;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Size)
	float m_capsuleHalfHeight;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Size)
	float m_health;
};

/**
 * 
 */
UCLASS()
class LUDUMGAME_API ACalfAgent : public ABasicAIAgent
{
	GENERATED_BODY()

	ACalfAgent();

public:

	void BeginPlay();

	void ChangeTeam(int32 newTeam) override;

	UFUNCTION(BlueprintCallable, Category=Size)
	void SetCalfSize(int32 sizeIndex);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Size)
	TArray<FCalfSizeSetup> m_sizes;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Size)
	int32 m_currentSize;

	UFUNCTION(BlueprintImplementableEvent, Category = Trumpet)
	void OnCalfTrumpetAttracted();

	virtual void KillAgent() override;

	virtual void ModifyHealth(float modificationAmount) override;
};