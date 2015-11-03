// copyright 2015 inbetweengames GBR

#pragma once

#include "BasicAIAgent.h"
#include "Hunter.generated.h"

UENUM(Blueprintable)
namespace EHunterState
{
	enum Type
	{
		RunBehaviours,
		IntoThrow,
		Throwing,
		AfterThrow,
		Feasting
	};
}

/**
 * 
 */
UCLASS()
class LUDUMGAME_API AHunter : public ABasicAIAgent
{
	GENERATED_BODY()
		
public:

	AHunter();

	virtual void Tick(float DeltaSeconds);

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintImplementableEvent, Category=Hunter)
	void OnHunterStateChanged(EHunterState::Type newState);
		
	EHunterState::Type m_currentState;
	
	UPROPERTY(Transient)
	AActor *m_throwTarget;

	virtual bool ShouldRunBehaviours() override
	{
		if (m_health <= 0.0f)
		{
			return false;
		}
		return m_currentState == EHunterState::RunBehaviours;
	}

	void SetState(EHunterState::Type newState);

	AActor *FindThrowTarget();
	void StartThrow(AActor *forTarget);

	void UpdateAnimation(float DeltaSeconds);

	int32 FindValidTarget(const TArray<FVector> &potentialTargets);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Throw)
	TSubclassOf<class AHunterProjectile> m_projectileClass;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Throw)
	float m_minTargetDistanceToStartThrow;

	bool IsPlayerInFearRadius();

	void ThrowProjectile();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Throw)
	float m_intoThrowTime;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Throw)
	float m_doingThrowTime;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Throw)
	float m_afterThrowTime;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Throw)
	float m_randomThrowCheckTimeMin;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Throw)
	float m_randomThrowCheckTimeMax;

	float m_currentStateTime;

	FAnimationSetup m_originalIdleAnim;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Throw)
	TArray<FAnimationSetup> m_feastAnims;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Throw)
	FAnimationSetup m_throwIntoAnim;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Throw)
	FAnimationSetup m_throwingAnim;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Throw)
	FAnimationSetup m_throwAfterAnim;

	float m_countdownForThrowCheck;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Panic)
	float m_feastRadius;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Panic)
	float m_panicOnDeathRadius;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Panic)
	int32 m_teamToChangetoOnPanic;

	void KillAgent() override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Panic)
	float m_chanceToPanic;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = VO)
	int32 m_hunterTeamSizeForMultipleHuntersVO;

};
