// copyright 2015 inbetweengames GBR

#pragma once

#include "GameFramework/Actor.h"
#include "HunterProjectile.generated.h"

UCLASS()
class LUDUMGAME_API AHunterProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHunterProjectile();
	
	UFUNCTION(BlueprintImplementableEvent, Category = Hunter)
	void OnProjectileHit();
		
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	void MoveInDirection(const FVector &direction);

	AActor * FindHitActor();
	int32 FindHitActor(const TArray<FVector> &potentialTargets, float targetRadius);

	FVector m_velocity;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Projectile)
	float m_projectileVelocity;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Projectile)
	float m_projectileDamage;

	float m_projectileHitRadius;
};
