// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
// copyright 2015 inbetweengames GBR

#pragma once

#include "AnimatedPlaneComponent.h"
#include "BasicAIAgent.generated.h"

UCLASS(config=Game)
class ABasicAIAgent : public AActor, public INavAgentInterface
{
	GENERATED_BODY()
		
public:

	ABasicAIAgent();

	virtual bool ShouldRunBehaviours()
	{
		return m_health > 0.0f;
	}

	virtual void Tick(float DeltaSeconds);

	UPROPERTY(Category = Character, EditAnywhere, BlueprintReadOnly)
	class UCapsuleComponent* m_capsuleComponent;

	virtual void BeginPlay() override;

	/** Returns Sprite subobject **/
	FORCEINLINE class UAnimatedPlaneComponent* GetSprite() const { return m_spriteComponent; }
	
	virtual FVector GetVelocity() const override;
	void SetVelocity(const FVector &newVelocity, const FRotator &newFacingDirection);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
	float m_maximumVelocity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
	float m_accelerationMagntiude;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
	float m_dampingFactor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
	float m_agentRotationSpeed;

	int32 m_currentAgentID;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category=Agent)
	int32 m_deathTeamID;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category=Agent)
	int32 m_teamID;

	UFUNCTION(BlueprintCallable, Category=Agent)
	virtual void ChangeTeam(int32 newTeam);

	UFUNCTION(BlueprintImplementableEvent, Category=Agent)
	void OnTeamChanged(int32 newTeam);

	virtual void ModifyHealth(float modificationAmount);


	UFUNCTION(BlueprintImplementableEvent, Category=Health)
	void OnDied();
	
	UFUNCTION(BlueprintImplementableEvent, Category=Health)
	void Mammoth_OnTakeDamage(float damage);

	/**
	*	Retrieves FNavAgentProperties expressing navigation props and caps of represented agent
	*	@NOTE the function will be renamed to GetNavAgentProperties in 4.8. Current name was introduced
	*		to help with deprecating old GetNavAgentProperties function
	*/
	virtual const FNavAgentProperties& GetNavAgentPropertiesRef() const;
	
	/** Properties that define how the component can move. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement Capabilities", meta=(DisplayName="Movement Capabilities", Keywords="Nav Agent"))
	FNavAgentProperties NavAgentProps;

	float CalcAgentRadius();

	bool IsAlive()
	{
		return m_health > 0.0f;
	}

	float GetHealth()
	{
		return m_health;
	}

	UFUNCTION(BlueprintImplementableEvent, Category=Spear)
	UBoxComponent *GetSpearBox();

protected:

	virtual void KillAgent();

	/** The main skeletal mesh associated with this Character (optional sub-object). */
	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UAnimatedPlaneComponent* m_spriteComponent;

	// The animation to play while running around
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Animations)
	FAnimationSetup m_walkAnimation;
	
	// The animation to play while idle (standing still)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
	FAnimationSetup m_idleAnimation;

	// The animation to play while dead
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
	TArray<FAnimationSetup> m_deathAnimations;

	int32 m_currentDeathAnimIndex;

	/** Called to choose the correct animation to play based on the character's movement state */
	virtual void UpdateAnimation(float DeltaSeconds);

	float m_health;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Movement)
	bool m_rotateInMovementDirection;

	FVector m_currentVelocity;

	bool m_wasGoingLeft;

	float m_timeGoingInOtherDirection;

	FVector m_positionLastStamp;
	float m_timeSinceLastStamp;
	float m_positionStampTime;
	float m_stampVelocity;
};

