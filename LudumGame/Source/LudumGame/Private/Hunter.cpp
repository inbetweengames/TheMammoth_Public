// copyright 2015 inbetweengames GBR

#include "LudumGame.h"
#include "Hunter.h"
#include "FlockingDataCache.h"
#include "Flocking/FearBehaviourComponent.h"
#include "Public/HunterProjectile.h"
#include "TP_TopDown/TP_TopDownCharacter.h"

AHunter::AHunter()
{
	m_minTargetDistanceToStartThrow = 500.0;

	m_currentStateTime = 0.0f;

	PrimaryActorTick.bCanEverTick = true;

	m_randomThrowCheckTimeMin = 1.0f;
	m_randomThrowCheckTimeMax = 2.0f;

	m_countdownForThrowCheck = FMath::FRandRange(m_randomThrowCheckTimeMin, m_randomThrowCheckTimeMax);

	m_panicOnDeathRadius = 1000.0f;
	m_teamToChangetoOnPanic = TEAM_HUNTERS_PANIC;

	m_chanceToPanic = 0.1f;

	m_hunterTeamSizeForMultipleHuntersVO = 8;

	m_feastRadius = 500.0f;
}


void AHunter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (m_health <= 0.0f)
	{
		return;
	}

	m_currentStateTime += DeltaSeconds;

	switch (m_currentState)
	{
		case EHunterState::RunBehaviours:
		{
			m_countdownForThrowCheck -= DeltaSeconds;
			if (m_countdownForThrowCheck <= 0.0f)
			{
				m_countdownForThrowCheck = FMath::FRandRange(m_randomThrowCheckTimeMin, m_randomThrowCheckTimeMax);
				if (!IsPlayerInFearRadius() && m_teamID != TEAM_HUNTERS_PANIC)
				{
					AActor * throwTarget = FindThrowTarget();
					if (throwTarget)
					{
						StartThrow(throwTarget);
						break;
					}
				}
			}

			FVector thisLocation = GetActorLocation();

			FYAgentTeamData &deadCalvesTeam = UFlockingDataCache::GetCacheChecked(this)->GetOrCreateTeamData(TEAM_CALVES_DEAD);
			bool isFeasting = false;
			for (FVector &deadCalf : deadCalvesTeam.m_locations)
			{
				FVector toCorpse = deadCalf - thisLocation;				
				if (toCorpse.Size() <= m_feastRadius)
				{
					m_wasGoingLeft = toCorpse.Y < 0.0f;
					isFeasting = true;
					break;
				}
			}

			if (isFeasting)
			{
				SetState(EHunterState::Feasting);
			}
			break;
		}
		case EHunterState::IntoThrow:
		{
			if (IsPlayerInFearRadius())
			{
				SetState(EHunterState::RunBehaviours);
			}
			else if (m_currentStateTime >= m_intoThrowTime)
			{
				SetState(EHunterState::Throwing);
			}
			break;
		}
		case EHunterState::Throwing:
		{
			if (m_currentStateTime >= m_doingThrowTime)
			{
				ThrowProjectile();
				SetState(EHunterState::AfterThrow);
			}
			break;
		}
		case EHunterState::AfterThrow:
		{
			if (m_currentStateTime >= m_afterThrowTime)
			{
				SetState(EHunterState::RunBehaviours);
			}
			break;
		}
	}
}

void AHunter::ThrowProjectile()
{
	if (m_projectileClass == nullptr || m_throwTarget == nullptr)
	{
		return;
	}

	FVector thisLoc = GetActorLocation();

	AHunterProjectile *thrownProj = Cast<AHunterProjectile>(GetWorld()->SpawnActor(m_projectileClass, &thisLoc));
	FVector toTarget = m_throwTarget->GetActorLocation() - GetActorLocation();
	toTarget.Z = 0.0f;
	toTarget.Normalize();

	thrownProj->MoveInDirection(toTarget);

}

int32 AHunter::FindValidTarget(const TArray<FVector> &potentialTargets)
{
	const FVector &thisLocation = GetActorLocation();

	float minDistSq = FMath::Square(m_minTargetDistanceToStartThrow);

	int32 closest = INDEX_NONE;
	float closestSq = MAX_FLT;
	for (int32 i = 0; i < potentialTargets.Num(); i++)
	{
		const FVector &target = potentialTargets[i];

		float distSq = FVector::DistSquared(thisLocation, target);
		if (distSq < closestSq && distSq < minDistSq)
		{
			closest = i;
			closestSq = distSq;
		}
	}

	return closest;
}

bool AHunter::IsPlayerInFearRadius()
{
	const FVector &thisLocation = GetActorLocation();

	UFlockingDataCache *cache = UFlockingDataCache::GetCacheChecked(this);

	const TArray<FVector> &playerLocations = cache->GetLocationsPlayer();

	// if player is too close, run away

	UFearBehaviourComponent *fearBeh = Cast<UFearBehaviourComponent>(cache->GetTeamData(TEAM_HUNTERS)->GetBehaviourOfType(UFearBehaviourComponent::StaticClass()));

	float fearRadius = fearBeh->m_fearRadius * 0.8f;
	float fearRadSq = FMath::Square(fearRadius);
	for (const FVector & playerLocation : playerLocations)
	{
		if (FVector::DistSquared(playerLocation, thisLocation) <= fearRadSq)
		{
			return true;
		}
	}

	return false;
}


AActor * AHunter::FindThrowTarget()
{
	const FVector &thisLocation = GetActorLocation();

	UFlockingDataCache *cache = UFlockingDataCache::GetCacheChecked(this);

	const TArray<FVector> &calfLocations = cache->GetTeamData(TEAM_CALVES)->m_locations;
	const TArray<FVector> &playerLocations = cache->GetLocationsPlayer();
	
	int32 targetCalf = FindValidTarget(calfLocations);
	AActor *targetActor = nullptr;
	if (targetCalf == INDEX_NONE)
	{
		int32 targetPlayer = FindValidTarget(playerLocations);
		if (targetPlayer != INDEX_NONE)
		{

			for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
			{
				AController *controller = *It;
				APawn *otherPawn = controller->GetPawn();
				if (otherPawn == nullptr)
				{
					continue;
				}

				if (controller->IsA(APlayerController::StaticClass()))
				{
					targetActor = controller->GetPawn();
				}
			}
		}
	}
	else
	{
		targetActor = cache->GetTeamData(TEAM_CALVES)->m_agents[targetCalf];
	}

	return targetActor;
}

void AHunter::StartThrow(AActor *forTarget)
{
	m_throwTarget = forTarget;
	SetState(EHunterState::IntoThrow);
}

void AHunter::SetState(EHunterState::Type newState)
{
	if (newState == m_currentState)
	{
		return;
	}

	m_currentStateTime = 0.0f;
	m_currentState = newState;

	if (m_currentState == EHunterState::RunBehaviours)
	{
		m_countdownForThrowCheck = FMath::FRandRange(m_randomThrowCheckTimeMin, m_randomThrowCheckTimeMax);
	}
	else if (m_currentState == EHunterState::Feasting && m_feastAnims.Num() > 0)
	{
		int32 feastAnim = FMath::RandRange(0, m_feastAnims.Num()-1);
		m_idleAnimation = m_feastAnims[feastAnim];
		m_walkAnimation = m_idleAnimation;
		m_positionLastStamp = GetActorLocation();
		m_stampVelocity = 0.0f;
	}

	OnHunterStateChanged(newState);
}

void AHunter::UpdateAnimation(float DeltaSeconds)
{
	if (m_health <= 0.0f)
	{
		Super::UpdateAnimation(DeltaSeconds);
		return;
	}

	FVector thisLocation = GetActorLocation();
	if (m_throwTarget)
	{
		FVector toTarget = m_throwTarget->GetActorLocation() - thisLocation;

		m_wasGoingLeft = toTarget.Y < 0.0f;
	}

	switch (m_currentState)
	{
	case EHunterState::RunBehaviours:
	case EHunterState::Feasting:
	{
		Super::UpdateAnimation(DeltaSeconds);
		break;
	}
	case EHunterState::IntoThrow:
	{
		m_spriteComponent->SetNewAnimation(m_throwIntoAnim, m_wasGoingLeft);
		break;
	}
	case EHunterState::Throwing:
	{
		m_spriteComponent->SetNewAnimation(m_throwingAnim, m_wasGoingLeft);
		break;
	}
	case EHunterState::AfterThrow:
	{
		m_spriteComponent->SetNewAnimation(m_throwAfterAnim, m_wasGoingLeft);
		break;
	}
	}
}

void AHunter::KillAgent()
{
	Super::KillAgent();

	UFlockingDataCache *cache = UFlockingDataCache::GetCacheChecked(this);
	FYAgentTeamData &thisTeam = cache->GetOrCreateTeamData(m_teamID);

	FVector thisLoc = GetActorLocation();

	for (int32 i = 0; i < thisTeam.m_agents.Num(); i++)
	{
		ABasicAIAgent *otherHunter = thisTeam.m_agents[i];
		if (otherHunter && otherHunter->IsAlive() && FVector::Dist(thisLoc, otherHunter->GetActorLocation()) <= m_panicOnDeathRadius)
		{
			float rand = FMath::FRand();
			if (rand <= m_chanceToPanic)
			{
				otherHunter->ChangeTeam(m_teamToChangetoOnPanic);

				otherHunter->SetLifeSpan(20.0f);
			}
		}
	}

	ATP_TopDownCharacter *playerPawn = Cast<ATP_TopDownCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn());
	if (playerPawn)
	{
		playerPawn->EnemyKilled();
	}

	SetLifeSpan(20.0f);
}

void AHunter::BeginPlay()
{
	Super::BeginPlay();

	if (UFlockingDataCache::GetNumAgentsInTeam(this, TEAM_HUNTERS) >= m_hunterTeamSizeForMultipleHuntersVO)
	{
		ATP_TopDownCharacter *playerPawn = Cast<ATP_TopDownCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn());
		if (playerPawn)
		{
			playerPawn->PlayVOEvent(ESystematicVOEvent::HuntersAppear);
		}
	}

	m_originalIdleAnim = m_idleAnimation;
}
