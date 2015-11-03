// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
// copyright 2015 inbetweengames GBR

#pragma once
#include "GameFramework/Character.h"
#include "AnimatedPlaneComponent.h"
#include "TP_TopDownCharacter.generated.h"

UENUM(Blueprintable, BlueprintType)
namespace ESystematicVOEvent
{
	enum Type
	{
		ChildrenAbandoned,
		ChildrenAllDead,
		ChildSmallDead,
		ChildMiddleDead,
		ChildBigDead,
		ChildFollowing,
		ChildrenFollowing,
		ChildTakesDamage,
		ChildrenTakeDamage,
		HuntersWithinRadius,
		PlayerDominating,
		HuntersAppear,
		PlayerInvincible
	};
}

UCLASS(Blueprintable)
class ATP_TopDownCharacter : public ACharacter
{
	GENERATED_BODY()

protected:

	// Name of the Sprite component
	static FName SpriteComponentName;

	void BeginPlay();

	/** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* TopDownCameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** The main skeletal mesh associated with this Character (optional sub-object). */
	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UAnimatedPlaneComponent* m_animatedSprite;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Animation)
	FAnimationSetup m_idleAnim;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Animation)
	FAnimationSetup m_walkAnim;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Animation)
	FAnimationSetup m_chargingAnim;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Animation)
	FAnimationSetup m_trumpetAnim;

	bool m_isGoingLeft;

	virtual void Tick(float DeltaSeconds) override;

	void UpdateAnimation();
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Charge)
	float m_chargeCooldownTime;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Charge)
	float m_chargeAttackDuration;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Charge)
	float m_chargeVelocityMultiplier;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Charge)
	float m_chargeDamageMultiplier;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Charge)
	float m_chargeDamageRadiusMultiplier;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Trumpet)
	float m_trumpetCooldownTime;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Trumpet)
	float m_trumpetFearRadiusMultiplier;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Trumpet)
	float m_trumpetFearWeightMultiplier;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Trumpet)
	float m_trumpetCalfAttractionWeightMultiplier;	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Trumpet)
	float m_trumpetCalfAttractionRadiusMultiplier;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Trumpet)
	float m_trumpetCalfSpeedMultiplier;	
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Trumpet)
	float m_trumpetActivationRadius;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Trumpet)
	float m_trumpetMovementStopTime;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Trumpet)
	float m_trumpetEffectTime;

	float m_originalMaxVelocity;
	
	UFUNCTION(BlueprintImplementableEvent, Category=Health)
	void OnCharge();
	
	UFUNCTION(BlueprintImplementableEvent, Category=Health)
	void OnChargeEnd();

	UFUNCTION(BlueprintImplementableEvent, Category=Health)
	void OnTrumpet();

	UFUNCTION(BlueprintImplementableEvent, Category = VO)
	void OnPlayVOEvent(ESystematicVOEvent::Type event);

public:

	UFUNCTION(BlueprintCallable, Category=Rendering)
	static bool IsFeatureLevelGLES(UObject *context);
	
	UFUNCTION(BlueprintImplementableEvent, Category = Spear)
	UBoxComponent *GetSpearBox();

	UFUNCTION(BlueprintImplementableEvent, Category = GameState)
	void OnGameStarted();

	FTimerHandle m_chargeTimerHandle;
	FTimerHandle m_trumpetMovementTimerHandle;
	FTimerHandle m_trumpetEffectTimerHandle;

	ATP_TopDownCharacter(const FObjectInitializer& ObjectInitializer);

	/** Returns TopDownCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	UFUNCTION(BlueprintCallable, Category=Input)
	void DoChargeAttack();

	UFUNCTION(BlueprintCallable, Category = Input)
	void DoTrumpet();

	UFUNCTION()
	void EndChargeAttack();

	UFUNCTION()
	void EndTrumpetMovementRemoval();

	UFUNCTION()
	void EndTrumpetEffect();

	UFUNCTION(BlueprintCallable, Category = Charge)
	bool IsCharging();

	UFUNCTION(BlueprintCallable, Category=Trumpet)
	bool IsTrumpeting();

	void EnemyKilled();
	void SpearHit();

	bool m_isTrumpetEffectActive;

	FVector m_lastInput;
	FVector m_chargeVelocity;

	void PlayVOEvent(ESystematicVOEvent::Type event);

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = VO)
	float m_hunterWithRadiusDist;

	TArray<ESystematicVOEvent::Type> m_playedVOs;
	TArray<ESystematicVOEvent::Type> m_queuedVOs;
		
	UPROPERTY(BlueprintReadOnly, Category=Charge)
	float m_timeUntilChargeAllowed;

	UPROPERTY(BlueprintReadOnly, Category = Trumpet)
	float m_timeUntilTrumpetAllowed;

	int32 m_enemiesKilled;
	int32 m_spearsHit;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = VO)
	int32 m_enemiesKilledForDominatingVO;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = VO)
	int32 m_spearHitsForInvincibleVO;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = VO)
	float m_childAbandonedVORadius;
};

