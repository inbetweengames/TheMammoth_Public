// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
// copyright 2015 inbetweengames GBR

#include "LudumGame.h"
#include "TP_TopDownPlayerController.h"
#include "AI/Navigation/NavigationSystem.h"
#include "TP_TopDownCharacter.h"

ATP_TopDownPlayerController::ATP_TopDownPlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Crosshairs;

	m_currentGameFlowState = EGameFlowState::None;

	m_wasGameEverStarted = false;

	m_isTrumpetUIOn = false;
	m_isChargeUIOn = false;

	m_touchReleaseStopMovingDelay = 0.3f;
}

void ATP_TopDownPlayerController::BeginPlay()
{
	Super::BeginPlay();

	SetGameFlowState(EGameFlowState::MainMenu);
}

struct SortedPlane
{
	float m_depth;

	UAnimatedPlaneComponent* m_plane;

	bool operator <(const SortedPlane &other) const
	{
		return other.m_depth < m_depth;
	}
};

void ATP_TopDownPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	// keep updating the destination every tick while desired
// 	if (bMoveToMouseCursor)
// 	{
// 		MoveToMouseCursor();
// 	}

	ATP_TopDownCharacter *controlledChar = Cast<ATP_TopDownCharacter>(GetPawn());
	if (m_isMovingFromTouch && m_currentGameFlowState == EGameFlowState::InGame &&
		controlledChar && !controlledChar->IsCharging())
	{
		// Trace to see what is under the touch location
		FHitResult HitResult;
		GetHitResultAtScreenPosition(m_touchLocationScreen, CurrentClickTraceChannel, true, HitResult);
		if (HitResult.bBlockingHit)
		{
			FVector thisLoc = controlledChar->GetActorLocation();
			FVector toMovePoint = HitResult.ImpactPoint - thisLoc;
			toMovePoint.Normalize();

			controlledChar->AddMovementInput(toMovePoint);
		}
	}

	ULevel *level = GetWorld()->GetLevel(0);
	TArray<SortedPlane> planesArray;

	for (AActor *actor : level->Actors)
	{
		if (actor == nullptr)
		{
			continue;
		}

		TArray<UActorComponent*> actorPlanes = actor->GetComponentsByClass(UAnimatedPlaneComponent::StaticClass());
		for (UActorComponent *comp : actorPlanes)
		{
			UAnimatedPlaneComponent * planeComp = CastChecked<UAnimatedPlaneComponent>(comp);

			if (planeComp->m_doDepthSort)
			{
				SortedPlane entry;
				entry.m_plane = planeComp;
				entry.m_depth = entry.m_plane->GetComponentLocation().X;
				planesArray.Add(entry);
			}
		}
	}

	planesArray.Sort();

	int32 prio = 1;
	for (SortedPlane &animPlane : planesArray)
	{
		animPlane.m_plane->SetTranslucentSortPriority(prio++);
	}
}

void ATP_TopDownPlayerController::SetupInputComponent()
{
	// set up gameplay key bindings
	Super::SetupInputComponent();

// 	InputComponent->BindAction("SetDestination", IE_Pressed, this, &ATP_TopDownPlayerController::OnSetDestinationPressed);
// 	InputComponent->BindAction("SetDestination", IE_Released, this, &ATP_TopDownPlayerController::OnSetDestinationReleased);

// 	// support touch devices 
	InputComponent->BindTouch(EInputEvent::IE_Pressed, this, &ATP_TopDownPlayerController::MoveToTouchLocation);
	InputComponent->BindTouch(EInputEvent::IE_Repeat, this, &ATP_TopDownPlayerController::MoveToTouchLocation);
	InputComponent->BindTouch(EInputEvent::IE_Released, this, &ATP_TopDownPlayerController::OnTouchReleased);

	InputComponent->BindAction("DoChargeAttack", IE_Pressed, this, &ATP_TopDownPlayerController::DoChargeAttack);
	InputComponent->BindAction("DoTrumpet", IE_Pressed, this, &ATP_TopDownPlayerController::DoTrumpet);
	InputComponent->BindAxis("MoveRight", this, &ATP_TopDownPlayerController::MoveRight);
	InputComponent->BindAxis("MoveForward", this, &ATP_TopDownPlayerController::MoveForward);

	InputComponent->BindAction("OpenOptions", IE_Pressed, this, &ATP_TopDownPlayerController::OpenOptions);

	InputComponent->BindAction("ForceTouch", IE_Pressed, this, &ATP_TopDownPlayerController::ForceTouch);
}

void ATP_TopDownPlayerController::DoTrumpet()
{
	switch (m_currentGameFlowState)
	{
	case EGameFlowState::Credits:
	{
		UGameplayStatics::GetGameMode(this)->RestartGame();
		break;
	}
	case EGameFlowState::InGame:
	{
		ATP_TopDownCharacter *controlledChar = Cast<ATP_TopDownCharacter>(GetPawn());
		if (controlledChar)
		{
			controlledChar->DoTrumpet();

			m_isTrumpetUIOn = true;
		}
		break;
	}
	case EGameFlowState::MainMenu:
	{
		SetGameFlowState(EGameFlowState::Intro);
		break;
	}
	}
}

void ATP_TopDownPlayerController::DoChargeAttack()
{
	switch (m_currentGameFlowState)
	{
	case EGameFlowState::Credits:
	{
		UGameplayStatics::GetGameMode(this)->RestartGame();
		break;
	}
	case EGameFlowState::InGame:
	{
		ATP_TopDownCharacter *controlledChar = Cast<ATP_TopDownCharacter>(GetPawn());
		if (controlledChar)
		{
			controlledChar->DoChargeAttack();

			m_isChargeUIOn = true;
		}
		break;
	}
	case EGameFlowState::MainMenu:
	{
		SetGameFlowState(EGameFlowState::Intro);
		break;
	}
	}


}

void ATP_TopDownPlayerController::MoveForward(float Value)
{
	if (m_currentGameFlowState != EGameFlowState::InGame)
	{
		return;
	}

	ATP_TopDownCharacter *controlledChar = Cast<ATP_TopDownCharacter>(GetPawn());
	if (controlledChar && !controlledChar->IsCharging())
	{
		controlledChar->AddMovementInput(FVector(1.f, 0.f, 0.f), Value);
	}
}

void ATP_TopDownPlayerController::MoveRight(float Value)
{
	if (m_currentGameFlowState != EGameFlowState::InGame)
	{
		return;
	}

	ATP_TopDownCharacter *controlledChar = Cast<ATP_TopDownCharacter>(GetPawn());
	if (controlledChar && !controlledChar->IsCharging())
	{
		controlledChar->AddMovementInput(FVector(0.f, 1.f, 0.f), Value);
	}	
}

void ATP_TopDownPlayerController::MoveToMouseCursor()
{
	if (m_currentGameFlowState != EGameFlowState::InGame)
	{
		return;
	}

	// Trace to see what is under the mouse cursor
	FHitResult Hit;
	GetHitResultUnderCursor(ECC_Visibility, false, Hit);

	if (Hit.bBlockingHit)
	{
		// We hit something, move there
		SetNewMoveDestination(Hit.ImpactPoint);
	}
}

void ATP_TopDownPlayerController::MoveToTouchLocation(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (m_currentGameFlowState != EGameFlowState::InGame)
	{
		return;
	}


	ATP_TopDownCharacter *controlledChar = Cast<ATP_TopDownCharacter>(GetPawn());
	if (m_currentGameFlowState != EGameFlowState::InGame ||
		!controlledChar || controlledChar->IsCharging() || controlledChar->IsTrumpeting())
	{
		m_isMovingFromTouch = false; 
		return;
	}

	m_touchLocationScreen = FVector2D(Location);
	m_isMovingFromTouch = true;

	GetWorld()->GetTimerManager().ClearTimer(m_stopMovingTimerHandle);
}

void ATP_TopDownPlayerController::OnTouchReleased(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (m_isMovingFromTouch)
	{
		GetWorld()->GetTimerManager().SetTimer(m_stopMovingTimerHandle, this, &ATP_TopDownPlayerController::StopMovingToTouchLocation, m_touchReleaseStopMovingDelay);
	}
}

void ATP_TopDownPlayerController::StopMovingToTouchLocation()
{
	m_isMovingFromTouch = false;
}

void ATP_TopDownPlayerController::SetNewMoveDestination(const FVector DestLocation)
{
	APawn* const Pawn = GetPawn();
	if (Pawn)
	{
		UNavigationSystem* const NavSys = GetWorld()->GetNavigationSystem();
		float const Distance = FVector::Dist(DestLocation, Pawn->GetActorLocation());

		// We need to issue move command only if far enough in order for walk animation to play correctly
		if (NavSys && (Distance > 120.0f))
		{
			NavSys->SimpleMoveToLocation(this, DestLocation);
		}
	}
}

void ATP_TopDownPlayerController::OnSetDestinationPressed()
{
	if (m_currentGameFlowState != EGameFlowState::InGame)
	{
		return;
	}

	// set flag to keep updating destination until released
	bMoveToMouseCursor = true;
}

void ATP_TopDownPlayerController::OnSetDestinationReleased()
{
	if (m_currentGameFlowState != EGameFlowState::InGame)
	{
		return;
	}

	// clear flag to indicate we should stop updating the destination
	bMoveToMouseCursor = false;
}

void ATP_TopDownPlayerController::SetGameFlowState(EGameFlowState::Type newState)
{
	if (newState == m_currentGameFlowState)
	{
		return;
	}

	m_currentGameFlowState = newState;
	if (m_currentGameFlowState == EGameFlowState::Options)
	{
		SetPause(true);
	}
	else
	{
		SetPause(false);
	}

	OnGameFlowStateChanged(newState);

	if (m_wasGameEverStarted == false && m_currentGameFlowState == EGameFlowState::Intro)
	{
		ATP_TopDownCharacter *playerChar = CastChecked<ATP_TopDownCharacter>(GetPawn());		
		playerChar->OnGameStarted();
		m_wasGameEverStarted = true;
	}
}

void ATP_TopDownPlayerController::OpenOptions()
{
	if (m_currentGameFlowState == EGameFlowState::InGame)
	{
		SetGameFlowState(EGameFlowState::Options);
	}
	else if (m_currentGameFlowState == EGameFlowState::Options)
	{
		SetGameFlowState(EGameFlowState::InGame);
	}
}

void ATP_TopDownPlayerController::ForceTouch()
{
	s_forceTouch = !s_forceTouch;
	OnForceTouch();
}

bool ATP_TopDownPlayerController::s_forceTouch = false;

bool ATP_TopDownPlayerController::IsMobilePlatform()
{
	if (s_forceTouch == true)
	{
		return true;
	}
	FString name = UGameplayStatics::GetPlatformName();
	return name.Contains(TEXT("android")) || name.Contains(TEXT("ios"));
}
