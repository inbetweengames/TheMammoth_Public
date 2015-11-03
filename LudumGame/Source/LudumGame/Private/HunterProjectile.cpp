// copyright 2015 inbetweengames GBR

#include "LudumGame.h"
#include "HunterProjectile.h"
#include "FlockingDataCache.h"
#include "BasicAIAgent.h"
#include "TP_TopDown/TP_TopDownCharacter.h"


// Sets default values
AHunterProjectile::AHunterProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	m_projectileDamage = 25.0f;

	m_projectileVelocity = 100.0f;

	InitialLifeSpan = 30.0f;
}

// Called when the game starts or when spawned
void AHunterProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	FVector origin, extent;
	GetActorBounds(false, origin, extent);

	m_projectileHitRadius = extent.GetMax();
}

// Called every frame
void AHunterProjectile::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	if (GetAttachParentActor())
	{
		return;
	}

	FVector deltaMove = DeltaTime * m_velocity;

	FVector newLocation = GetActorLocation() + deltaMove;
	SetActorLocation(newLocation, false);

	AActor *hitActor = FindHitActor();
	if (hitActor)
	{
		ABasicAIAgent *hitAgent = Cast<ABasicAIAgent>(hitActor);
		bool doAttach = false;
		UBoxComponent *attachBox = nullptr;
		if (hitAgent && hitAgent->m_teamID == TEAM_CALVES)
		{
			attachBox = hitAgent->GetSpearBox();

			hitAgent->ModifyHealth(-m_projectileDamage);
			doAttach = true;
		}
		else if (ATP_TopDownCharacter *playerChar = Cast<ATP_TopDownCharacter>(hitActor))
		{
			attachBox = playerChar->GetSpearBox();

			playerChar->SpearHit();
			doAttach = true;
		}

		if (attachBox)
		{
			FVector boxExtents = attachBox->GetScaledBoxExtent();
			FVector boxOrigin = attachBox->GetComponentLocation();

			FBox box(boxOrigin - boxExtents, boxOrigin + boxExtents);

			FVector attachPos = FMath::RandPointInBox(box);
			SetActorLocation(attachPos);
		}

		if (doAttach)
		{
		//	Destroy();
			SetLifeSpan(20.0f);

			GetRootComponent()->AttachTo(hitActor->GetRootComponent(), NAME_None, EAttachLocation::KeepWorldPosition);

			OnProjectileHit();
		}
	}

}



int32 AHunterProjectile::FindHitActor(const TArray<FVector> &potentialTargets, float targetRadius)
{
	FVector thisLocation = GetActorLocation();

	float minDistSq = FMath::Square(m_projectileHitRadius + targetRadius);

	int32 closest = INDEX_NONE;
	float closestSq = MAX_FLT;
	for (int32 i = 0; i < potentialTargets.Num(); i++)
	{
		const FVector &target = potentialTargets[i];
		thisLocation.Z = target.Z;

		float distSq = FVector::DistSquared(thisLocation, target);
		if (distSq < closestSq && distSq < minDistSq)
		{
			closest = i;
			closestSq = distSq;
		}
	}

	return closest;
}

AActor * AHunterProjectile::FindHitActor()
{
	const FVector &thisLocation = GetActorLocation();

	UFlockingDataCache *cache = UFlockingDataCache::GetCacheChecked(this);
	
	ACharacter *playerCharacter = nullptr;
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
			playerCharacter = Cast<ACharacter>(controller->GetPawn());
		}
	}
	
	const TArray<FVector> &calfLocations = cache->GetTeamData(TEAM_CALVES)->m_locations;
	const TArray<FVector> &playerLocations = cache->GetLocationsPlayer();

	int32 targetCalf = FindHitActor(calfLocations, cache->GetTeamData(TEAM_CALVES)->m_agentRadius * 0.5f);
	AActor *targetActor = nullptr;
	if (targetCalf == INDEX_NONE)
	{
		int32 targetPlayer = FindHitActor(playerLocations, 0.5f * playerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius());
		if (targetPlayer != INDEX_NONE)
		{
			targetActor = playerCharacter;
		}
	}
	else
	{
		targetActor = cache->GetOrCreateTeamData(TEAM_CALVES).m_agents[targetCalf];
	}
	
	return targetActor;
}

void AHunterProjectile::MoveInDirection(const FVector &direction)
{
	m_velocity = direction * m_projectileVelocity;

	USceneComponent *hopefullySprite = GetRootComponent()->GetChildComponent(0);
	if (hopefullySprite != nullptr)
	{
		FRotator thisRotation = hopefullySprite->RelativeRotation;

		FRotator directionRotator = direction.Rotation();

		thisRotation.Yaw = directionRotator.Yaw;

		hopefullySprite->SetRelativeRotation(thisRotation);
	}
}

