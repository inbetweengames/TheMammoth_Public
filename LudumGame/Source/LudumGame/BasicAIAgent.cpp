// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
// copyright 2015 inbetweengames GBR

#include "LudumGame.h"
#include "BasicAIAgent.h"
#include "Private/FlockingDataCache.h"
#include "Private/Flocking/AgentBehaviourComponent.h"

//////////////////////////////////////////////////////////////////////////
// ABasicAIAgent

ABasicAIAgent::ABasicAIAgent()
{
	PrimaryActorTick.bCanEverTick = true;

	m_capsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	SetRootComponent(m_capsuleComponent);

	static FName CapsuleProfileName(TEXT("Agent"));
	m_capsuleComponent->SetCollisionProfileName(CapsuleProfileName);
	m_capsuleComponent->bCanEverAffectNavigation = false;

	// Try to create the sprite component
	m_spriteComponent = CreateOptionalDefaultSubobject<UAnimatedPlaneComponent>(TEXT("SpriteComponent"));
	if (m_spriteComponent)
	{
		m_spriteComponent->AlwaysLoadOnClient = true;
		m_spriteComponent->AlwaysLoadOnServer = true;
		m_spriteComponent->bOwnerNoSee = false;
		m_spriteComponent->bAffectDynamicIndirectLighting = true;
		m_spriteComponent->PrimaryComponentTick.TickGroup = TG_PrePhysics;
		m_spriteComponent->AttachParent = m_capsuleComponent;
		static FName CollisionProfileName(TEXT("NoCollision"));
		m_spriteComponent->SetCollisionProfileName(CollisionProfileName);
		m_spriteComponent->bGenerateOverlapEvents = false;

		m_spriteComponent->RelativeRotation = FRotator(0.0f, 90.0f, 90.0f);
		m_spriteComponent->bCanEverAffectNavigation = false;
	}

	m_maximumVelocity = 200.0f;
	m_accelerationMagntiude = 40000.0f;
	m_dampingFactor = 0.5f;
	m_agentRotationSpeed = 180.0f;

	m_currentAgentID = INDEX_NONE;

	m_health = 100.0f;

	NavAgentProps = FNavAgentProperties::DefaultProperties;
	NavAgentProps.AgentRadius = CalcAgentRadius();

	m_currentDeathAnimIndex = 0;

	m_positionStampTime = 0.5f;
}


void ABasicAIAgent::BeginPlay()
{
	Super::BeginPlay();

	UFlockingDataCache::GetCacheChecked(this)->AddAIAgent(this);

	NavAgentProps.AgentRadius = CalcAgentRadius();
	NavAgentProps.AgentHeight = m_capsuleComponent->GetScaledCapsuleHalfHeight() * 2.0f;

	m_positionLastStamp = GetActorLocation();
	m_timeSinceLastStamp = 0.0f;
	m_stampVelocity = 0.0f;
}

float ABasicAIAgent::CalcAgentRadius()
{
	return m_capsuleComponent->GetScaledCapsuleRadius();
}

FVector ABasicAIAgent::GetVelocity() const
{
	return m_currentVelocity;
}

void ABasicAIAgent::SetVelocity(const FVector &newVelocity, const FRotator &newFacingDirection)
{
	m_currentVelocity = newVelocity;
	
	if (m_rotateInMovementDirection)
	{
		SetActorRotation(newFacingDirection);
	}
}

void ABasicAIAgent::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	m_timeSinceLastStamp += DeltaSeconds;
	if (m_timeSinceLastStamp > m_positionStampTime)
	{
		FVector currPos = GetActorLocation();
		float distSinceLastStamp = FVector::Dist(currPos, m_positionLastStamp);
		m_stampVelocity = distSinceLastStamp / m_timeSinceLastStamp;
		m_timeSinceLastStamp -= m_positionStampTime;
		m_positionLastStamp = currPos;
	}

	UpdateAnimation(DeltaSeconds);
}

void ABasicAIAgent::UpdateAnimation(float DeltaSeconds)
{
	if (m_health <= 0.0f)
	{
		if (m_deathAnimations.IsValidIndex(m_currentDeathAnimIndex))
		{
			m_spriteComponent->SetNewAnimation(m_deathAnimations[m_currentDeathAnimIndex], m_wasGoingLeft);
		}
		else
		{
			m_spriteComponent->SetNewAnimation(m_idleAnimation, m_wasGoingLeft);
		}
		return;
	}

	float velocityAbs = m_stampVelocity;
	const float idleVelocityThreshold = 20.0f;
	const float otherDirectionTimeThreshold = 0.5f;

	if (velocityAbs <= idleVelocityThreshold)
	{
		m_spriteComponent->SetNewAnimation(m_idleAnimation, m_wasGoingLeft);
	}
	else
	{
		bool isGoingLeft = m_currentVelocity.Y < 0.0f;
		if (isGoingLeft != m_wasGoingLeft)
		{
			m_timeGoingInOtherDirection += DeltaSeconds;
			if (m_timeGoingInOtherDirection >= otherDirectionTimeThreshold)
			{
				m_wasGoingLeft = isGoingLeft;
				m_timeGoingInOtherDirection = 0.0f;
			}
		}
		else
		{
			m_timeGoingInOtherDirection = 0.0f;
		}
		
		m_spriteComponent->SetNewAnimation(m_walkAnimation, m_wasGoingLeft);
	}
}
void ABasicAIAgent::ChangeTeam(int32 newTeam)
{
	if (m_teamID == newTeam)
	{
		return;
	}

	UFlockingDataCache::GetCacheChecked(this)->QueueTeamChange(this, newTeam);
}

void ABasicAIAgent::ModifyHealth(float modificationAmount)
{
	bool wasAlive = m_health > 0.0f;
	if (!wasAlive)
	{
		return;
	}

	m_health += modificationAmount;

	if (modificationAmount < 0.0f)
	{
		Mammoth_OnTakeDamage(-modificationAmount);
	}
	
	if (m_health <= 0.0f)
	{
		KillAgent();
	}
}

void ABasicAIAgent::KillAgent()
{
	UFlockingDataCache::GetCacheChecked(this)->QueueTeamChange(this, m_deathTeamID);

	m_currentVelocity = FVector::ZeroVector;
	if (m_deathAnimations.Num() > 0)
	{
		m_currentDeathAnimIndex = FMath::RandRange(0, m_deathAnimations.Num() - 1);
	}

	UpdateAnimation(0.0f);

	OnDied();
}

const FNavAgentProperties& ABasicAIAgent::GetNavAgentPropertiesRef() const
{
	return NavAgentProps;
}
