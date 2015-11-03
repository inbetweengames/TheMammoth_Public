// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
// copyright 2015 inbetweengames GBR

#pragma once
#include "GameFramework/PlayerController.h"
#include "TP_TopDownPlayerController.generated.h"

UENUM(Blueprintable)
namespace EGameFlowState
{
	enum Type
	{
		None,
		MainMenu,
		Intro,
		InGame,
		Options,
		Credits
	};
}

UCLASS()
class ATP_TopDownPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ATP_TopDownPlayerController();

protected:
	/** True if the controlled character should navigate to the mouse cursor. */
	uint32 bMoveToMouseCursor : 1;

	// Begin PlayerController interface
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetupInputComponent() override;
	// End PlayerController interface

	/** Navigate player to the current mouse cursor location. */
	void MoveToMouseCursor();

	/** Navigate player to the current touch location. */
	void MoveToTouchLocation(const ETouchIndex::Type FingerIndex, const FVector Location);

	void OnTouchReleased(const ETouchIndex::Type FingerIndex, const FVector Location);

	void StopMovingToTouchLocation();

	/** Navigate player to the given world location. */
	void SetNewMoveDestination(const FVector DestLocation);

	/** Input handlers for SetDestination action. */
	void OnSetDestinationPressed();
	void OnSetDestinationReleased();

	void MoveRight(float Value);
	void MoveForward(float Value);

	void DoChargeAttack();
	void DoTrumpet();

	void OpenOptions();

	UPROPERTY(BlueprintReadWrite, Category = GameState)
	bool m_isTrumpetUIOn;
	UPROPERTY(BlueprintReadWrite, Category = GameState)
	bool m_isChargeUIOn;

public:

	UFUNCTION(BlueprintPure, BlueprintCallable, Category = Platform)
	static bool IsMobilePlatform();
	
	UFUNCTION(BlueprintImplementableEvent, Category=VO)
	bool IsVOPlaying();

	UFUNCTION(BlueprintCallable, Category=GameState)
	void SetGameFlowState(EGameFlowState::Type newState);

	UFUNCTION(BlueprintImplementableEvent, Category = GameState)
	void OnGameFlowStateChanged(EGameFlowState::Type newState);

	void BeginPlay() override;
	UPROPERTY(BlueprintReadOnly, Category = GameState)
	TEnumAsByte<EGameFlowState::Type> m_currentGameFlowState;

	bool m_wasGameEverStarted;

	bool m_isMovingFromTouch;

	FTimerHandle m_stopMovingTimerHandle;

	FVector2D m_touchLocationScreen;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Input)
	float m_touchReleaseStopMovingDelay;

	void ForceTouch();
	static bool s_forceTouch;

	UFUNCTION(BlueprintImplementableEvent, Category=Input)
	void OnForceTouch();
};


