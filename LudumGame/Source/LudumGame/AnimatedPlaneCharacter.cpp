// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
// copyright 2015 inbetweengames GBR

#include "LudumGame.h"
#include "AnimatedPlaneCharacter.h"
#include "AnimatedPlaneComponent.h"

FName AAnimatedPlaneCharacter::SpriteComponentName(TEXT("CharacterSprite"));

//////////////////////////////////////////////////////////////////////////
// AAnimatedPlaneCharacter

AAnimatedPlaneCharacter::AAnimatedPlaneCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.DoNotCreateDefaultSubobject(ACharacter::MeshComponentName))
{
	// Try to create the sprite component
	m_animatedSprite = CreateOptionalDefaultSubobject<UAnimatedPlaneComponent>(AAnimatedPlaneCharacter::SpriteComponentName);
	if (m_animatedSprite)
	{
		m_animatedSprite->AlwaysLoadOnClient = true;
		m_animatedSprite->AlwaysLoadOnServer = true;
		m_animatedSprite->bOwnerNoSee = false;
		m_animatedSprite->bAffectDynamicIndirectLighting = true;
		m_animatedSprite->PrimaryComponentTick.TickGroup = TG_PrePhysics;
		m_animatedSprite->AttachParent = GetCapsuleComponent();
		static FName CollisionProfileName(TEXT("CharacterMesh"));
		m_animatedSprite->SetCollisionProfileName(CollisionProfileName);
		m_animatedSprite->bGenerateOverlapEvents = false;

		m_animatedSprite->RelativeRotation = FRotator(90.0f, 0.0f, 0.0f);
	}
	
	// Don't rotate when the controller rotates.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// Create a camera boom attached to the root (capsule)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->AttachTo(RootComponent);
	CameraBoom->bAbsoluteRotation = true; // Rotation of the character should not affect rotation of boom
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->TargetArmLength = 500.f;
	CameraBoom->SocketOffset = FVector(0.f, 0.f, 75.f);
	CameraBoom->RelativeRotation = FRotator(0.f, 180.f, 0.f);

	// Create a camera and attach to boom
	SideViewCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("SideViewCamera"));
	SideViewCameraComponent->AttachTo(CameraBoom, USpringArmComponent::SocketName);
	SideViewCameraComponent->bUsePawnControlRotation = false; // We don't want the controller rotating the camera

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Face in the direction we are moving..
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->GravityScale = 2.f;
	GetCharacterMovement()->AirControl = 0.80f;
	GetCharacterMovement()->JumpZVelocity = 1000.f;
	GetCharacterMovement()->GroundFriction = 3.f;
	GetCharacterMovement()->MaxWalkSpeed = 600.f;
	GetCharacterMovement()->MaxFlySpeed = 600.f;
}

void AAnimatedPlaneCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (!IsPendingKill())
	{
		if (m_animatedSprite)
		{
			// force animation tick after movement component updates
			if (m_animatedSprite->PrimaryComponentTick.bCanEverTick && GetCharacterMovement())
			{
				m_animatedSprite->PrimaryComponentTick.AddPrerequisite(GetCharacterMovement(), GetCharacterMovement()->PrimaryComponentTick);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AAnimatedPlaneCharacter::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	// set up gameplay key bindings
	InputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	InputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	InputComponent->BindAxis("MoveRight", this, &AAnimatedPlaneCharacter::MoveRight);
}

void AAnimatedPlaneCharacter::MoveRight(float Value)
{
	// add movement in that direction
	AddMovementInput(FVector(0.f, -1.f, 0.f), Value);
}

void AAnimatedPlaneCharacter::UpdateAnimation()
{

	switch (GetCharacterMovement()->MovementMode)
	{
	case MOVE_Falling:
	case MOVE_Flying:
	{
		m_animatedSprite->SetNewAnimation(m_fallingAnim);
		break;
	}
	default:
	{
		const FVector &velocity = GetCharacterMovement()->Velocity;
		float velocityAbs = velocity.Size();
		if (FMath::IsNearlyZero(velocityAbs))
		{
			m_animatedSprite->SetNewAnimation(m_idleAnim);
		}
		else
		{
			bool isGoingRight = velocity.Y < 0.0f;
			if (isGoingRight)
			{
				m_animatedSprite->SetNewAnimation(m_walkAnimR);
			}
			else
			{
				m_animatedSprite->SetNewAnimation(m_walkAnimL);
			}
		}
		break;
	}
	}
}

void AAnimatedPlaneCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateAnimation();
}
