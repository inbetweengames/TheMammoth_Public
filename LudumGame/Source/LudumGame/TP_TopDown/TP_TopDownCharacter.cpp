// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
// copyright 2015 inbetweengames GBR

#include "LudumGame.h"
#include "TP_TopDownCharacter.h"
#include "AnimatedPlaneComponent.h"
#include "Private/FlockingDataCache.h"
#include "Private/Flocking/FearBehaviourComponent.h"
#include "Private/Flocking/PlayerFollowingBehaviourComponent.h"
#include "Private/TeamDamageRadiusComponent.h"
#include "BasicAIAgent.h"
#include "TP_TopDownPlayerController.h"
#include "CalfAgent.h"

FName ATP_TopDownCharacter::SpriteComponentName(TEXT("CharacterSprite"));

ATP_TopDownCharacter::ATP_TopDownCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.DoNotCreateDefaultSubobject(ACharacter::MeshComponentName))
{
	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Try to create the sprite component
	m_animatedSprite = CreateOptionalDefaultSubobject<UAnimatedPlaneComponent>(ATP_TopDownCharacter::SpriteComponentName);
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

	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Rotate character to moving direction
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	// Create a camera boom...
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->AttachTo(RootComponent);
	CameraBoom->bAbsoluteRotation = true; // Don't want arm to rotate when character does
	CameraBoom->TargetArmLength = 800.f;
	CameraBoom->RelativeRotation = FRotator(-60.f, 0.f, 0.f);
	CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level

	// Create a camera...
	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCameraComponent->AttachTo(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	m_chargeAttackDuration = 2.0f;
	m_chargeVelocityMultiplier = 4.0f;
	m_chargeDamageMultiplier = 10.0f;
	m_chargeDamageRadiusMultiplier = 1.0f;

	m_chargeCooldownTime = 5.0f;
	m_trumpetCooldownTime = 5.0f;

	m_trumpetFearRadiusMultiplier = 1.5f;
	m_trumpetFearWeightMultiplier = 3.0f;

	m_trumpetCalfAttractionWeightMultiplier = 5.0f;
	m_trumpetCalfAttractionRadiusMultiplier = 0.5f;

	m_trumpetActivationRadius = 1000.0f;

	m_trumpetMovementStopTime = 1.0f;
	m_trumpetEffectTime = 7.5;

	m_trumpetCalfSpeedMultiplier = 3.0f;

	m_isTrumpetEffectActive = false;

	m_timeUntilChargeAllowed = 0.0f;

	m_hunterWithRadiusDist = 1500.0f;
	m_enemiesKilledForDominatingVO = 15;
	m_spearHitsForInvincibleVO = 5;
	m_childAbandonedVORadius = 5000.0f;
}

void ATP_TopDownCharacter::BeginPlay()
{
	Super::BeginPlay();

	m_originalMaxVelocity = GetCharacterMovement()->MaxWalkSpeed;
}

void ATP_TopDownCharacter::Tick(float DeltaSeconds)
{
	FVector currentInputVector = LastControlInputVector;
	if (!currentInputVector.IsNearlyZero())
	{
		m_lastInput = currentInputVector;
	}

	Super::Tick(DeltaSeconds);

	UpdateAnimation();

	if (IsCharging())
	{
		AddMovementInput(m_chargeVelocity);
	}
	else
	{
		m_timeUntilChargeAllowed -= DeltaSeconds;
	}

	if (!m_isTrumpetEffectActive)
	{
		m_timeUntilTrumpetAllowed -= DeltaSeconds;
	}

	if (!m_playedVOs.Contains(ESystematicVOEvent::HuntersWithinRadius))
	{
		UFlockingDataCache *cache = UFlockingDataCache::GetCacheChecked(this);

		FYAgentTeamData &teamData = cache->GetOrCreateTeamData(TEAM_HUNTERS);

		FVector thisLoc = GetActorLocation();

		float distSq = FMath::Square(m_hunterWithRadiusDist);

		for (const FVector &hunterLoc : teamData.m_locations)
		{
			if (FVector::DistSquared(hunterLoc, thisLoc) <= distSq)
			{
				PlayVOEvent(ESystematicVOEvent::HuntersWithinRadius);
				break;
			}
		}
	}

	if (!m_playedVOs.Contains(ESystematicVOEvent::ChildrenAbandoned))
	{
		UFlockingDataCache *cache = UFlockingDataCache::GetCacheChecked(this);

		FYAgentTeamData &teamData = cache->GetOrCreateTeamData(TEAM_CALVES);

		FVector thisLoc = GetActorLocation();

		float distSq = FMath::Square(m_childAbandonedVORadius);

		for (const FVector &childLoc : teamData.m_locations)
		{
			if (FVector::DistSquared(childLoc, thisLoc) >= distSq)
			{
				PlayVOEvent(ESystematicVOEvent::ChildrenAbandoned);
				break;
			}
		}
	}

	if (m_queuedVOs.Num() > 0)
	{
		ATP_TopDownPlayerController *pC = Cast<ATP_TopDownPlayerController>(GetController());
		if (pC && !pC->IsVOPlaying())
		{
			auto event = m_queuedVOs[0];
			m_queuedVOs.RemoveAt(0);
			PlayVOEvent(event);
		}
	}
}

void ATP_TopDownCharacter::UpdateAnimation()
{
	const FVector &velocity = GetCharacterMovement()->Velocity;
	float velocityAbs = velocity.Size();
	const float idleVelocityThreshold = 5.0f;

	bool isIdling = velocityAbs <= idleVelocityThreshold;
	if (!isIdling)
	{
		m_isGoingLeft = velocity.Y < 0.0f;
	}

	if (IsCharging())
	{
		m_animatedSprite->SetNewAnimation(m_chargingAnim, m_isGoingLeft);
	}
	else if (IsTrumpeting())
	{
		m_animatedSprite->SetNewAnimation(m_trumpetAnim, m_isGoingLeft);
	}
	else if (isIdling)
	{
		m_animatedSprite->SetNewAnimation(m_idleAnim, m_isGoingLeft);
	}
	else
	{
		m_animatedSprite->SetNewAnimation(m_walkAnim, m_isGoingLeft);
	}
}

bool ATP_TopDownCharacter::IsCharging()
{
	return GetWorld()->GetTimerManager().IsTimerActive(m_chargeTimerHandle);
}

bool ATP_TopDownCharacter::IsTrumpeting()
{
	return GetWorld()->GetTimerManager().IsTimerActive(m_trumpetMovementTimerHandle);
}

void ATP_TopDownCharacter::DoChargeAttack()
{
	if (IsCharging() || m_timeUntilChargeAllowed > 0.0f)
	{
		return;
	}
	
	m_timeUntilChargeAllowed = m_chargeCooldownTime;

	// first use last input
	if (!m_lastInput.IsNearlyZero())
	{
		m_chargeVelocity = m_lastInput;
		m_chargeVelocity.Z = 0.0f;
		m_chargeVelocity.Normalize();
	}
	else
	{
		// no input: use current velocity
		m_chargeVelocity = GetCharacterMovement()->Velocity;
		m_chargeVelocity.Z = 0.0f;
		m_chargeVelocity.Normalize();
		if (m_chargeVelocity.IsNearlyZero())
		{
			// no velocity: use current facing direction
			if (m_isGoingLeft)
			{
				m_chargeVelocity = FVector(0.0f, -1.0f, 0.0f);
			}
			else
			{
				m_chargeVelocity = FVector(0.0f, 1.0f, 0.0f);
			}
		}
	}

	GetWorld()->GetTimerManager().SetTimer(m_chargeTimerHandle, this, &ATP_TopDownCharacter::EndChargeAttack, m_chargeAttackDuration);

	if (m_chargeVelocityMultiplier > 0.0f)
	{
		GetCharacterMovement()->MaxWalkSpeed *= m_chargeVelocityMultiplier;
	}

	UTeamDamageRadiusComponent *damageComp = FindComponentByClass<UTeamDamageRadiusComponent>();
	damageComp->m_damagePerSecond *= m_chargeDamageMultiplier;
	damageComp->m_damageRadius *= m_chargeDamageRadiusMultiplier;

	OnCharge();
}

void ATP_TopDownCharacter::EndChargeAttack()
{
	GetCharacterMovement()->MaxWalkSpeed = m_originalMaxVelocity;
	
	UTeamDamageRadiusComponent *damageComp = FindComponentByClass<UTeamDamageRadiusComponent>();
	damageComp->m_damagePerSecond /= m_chargeDamageMultiplier;
	damageComp->m_damageRadius /= m_chargeDamageRadiusMultiplier;

	OnChargeEnd();
}

void ATP_TopDownCharacter::DoTrumpet()
{
	if (IsTrumpeting() || m_timeUntilTrumpetAllowed > 0.0f)
	{
		return;
	}

	GetCharacterMovement()->MaxWalkSpeed = 0.0f;

	UFlockingDataCache *cache = UFlockingDataCache::GetCacheChecked(this);

	if (!m_isTrumpetEffectActive)
	{
		UFearBehaviourComponent *fearBeh = Cast<UFearBehaviourComponent>(cache->GetTeamData(TEAM_HUNTERS)->GetBehaviourOfType(UFearBehaviourComponent::StaticClass()));

		fearBeh->m_fearRadius *= m_trumpetFearRadiusMultiplier;
		fearBeh->m_behaviourWeight *= m_trumpetFearWeightMultiplier;

		FYAgentTeamData &calfTeam = cache->GetOrCreateTeamData(TEAM_CALVES);
		UPlayerFollowingBehaviourComponent *followBeh = Cast<UPlayerFollowingBehaviourComponent>(calfTeam.GetBehaviourOfType(UPlayerFollowingBehaviourComponent::StaticClass()));

		followBeh->m_behaviourWeight *= m_trumpetCalfAttractionWeightMultiplier;
		followBeh->m_minRadius *= m_trumpetCalfAttractionRadiusMultiplier;

		for (ABasicAIAgent *agent : calfTeam.m_agents)
		{
			agent->m_maximumVelocity *= m_trumpetCalfSpeedMultiplier;

			ACalfAgent *calf = Cast<ACalfAgent>(agent);
			if (calf)
			{
				calf->OnCalfTrumpetAttracted();
			}
		}

		m_isTrumpetEffectActive = true;
	}

	GetWorld()->GetTimerManager().SetTimer(m_trumpetMovementTimerHandle, this, &ATP_TopDownCharacter::EndTrumpetMovementRemoval, m_trumpetMovementStopTime);
	GetWorld()->GetTimerManager().SetTimer(m_trumpetEffectTimerHandle, this, &ATP_TopDownCharacter::EndTrumpetEffect, m_trumpetEffectTime);

	OnTrumpet();

	FYAgentTeamData &unactiavtedTeam = cache->GetOrCreateTeamData(TEAM_CALVES_UNACTIVATED);
	FVector thisLoc = GetActorLocation();
	for (ABasicAIAgent *agent : unactiavtedTeam.m_agents)
	{
		if (FVector::Dist(thisLoc, agent->GetActorLocation()) <= m_trumpetActivationRadius)
		{
			agent->ChangeTeam(TEAM_CALVES);

			agent->m_maximumVelocity *= m_trumpetCalfSpeedMultiplier;

			ACalfAgent *calf = Cast<ACalfAgent>(agent);
			if (calf)
			{
				calf->OnCalfTrumpetAttracted();
			}
		}
	}

	m_timeUntilTrumpetAllowed = m_trumpetCooldownTime;
}

void ATP_TopDownCharacter::EndTrumpetMovementRemoval()
{
	GetCharacterMovement()->MaxWalkSpeed = m_originalMaxVelocity;
}

void ATP_TopDownCharacter::EndTrumpetEffect()
{
	UFlockingDataCache *cache = UFlockingDataCache::GetCacheChecked(this);

	UFearBehaviourComponent *fearBeh = Cast<UFearBehaviourComponent>(cache->GetTeamData(TEAM_HUNTERS)->GetBehaviourOfType(UFearBehaviourComponent::StaticClass()));

	fearBeh->m_fearRadius /= m_trumpetFearRadiusMultiplier;
	fearBeh->m_behaviourWeight /= m_trumpetFearWeightMultiplier;

	FYAgentTeamData &calfTeam = cache->GetOrCreateTeamData(TEAM_CALVES);
	UPlayerFollowingBehaviourComponent *followBeh = Cast<UPlayerFollowingBehaviourComponent>(calfTeam.GetBehaviourOfType(UPlayerFollowingBehaviourComponent::StaticClass()));

	followBeh->m_behaviourWeight /= m_trumpetCalfAttractionWeightMultiplier;
	followBeh->m_minRadius /= m_trumpetCalfAttractionRadiusMultiplier;

	for (ABasicAIAgent *calf : calfTeam.m_agents)
	{
		calf->m_maximumVelocity /= m_trumpetCalfSpeedMultiplier;
	}

	m_isTrumpetEffectActive = false;
}

void ATP_TopDownCharacter::PlayVOEvent(ESystematicVOEvent::Type event)
{
	if (m_playedVOs.Contains(event))
	{
		return;
	}

	ATP_TopDownPlayerController *pC = Cast<ATP_TopDownPlayerController>(GetController());
	if (pC && pC->IsVOPlaying())
	{
		if (!m_queuedVOs.Contains(event))
		{
			m_queuedVOs.Add(event);
		}
		return;
	}

	OnPlayVOEvent(event);
	m_playedVOs.Add(event);
}

void ATP_TopDownCharacter::EnemyKilled()
{
	m_enemiesKilled++;
	if (m_enemiesKilled >= m_enemiesKilledForDominatingVO)
	{
		PlayVOEvent(ESystematicVOEvent::PlayerDominating);
	}
}

void ATP_TopDownCharacter::SpearHit()
{
	m_spearsHit++;
	if (m_spearsHit >= m_spearHitsForInvincibleVO)
	{
		PlayVOEvent(ESystematicVOEvent::PlayerInvincible);
	}
}

bool ATP_TopDownCharacter::IsFeatureLevelGLES(UObject *context)
{
	if (context == nullptr)
	{
		return false;
	}

	UWorld *world = context->GetWorld();
	if (world->FeatureLevel == ERHIFeatureLevel::ES2 || world->FeatureLevel == ERHIFeatureLevel::ES3_1)
	{
		return true;
	}
	else
	{
		return false;
	}
}
