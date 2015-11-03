// copyright 2015 inbetweengames GBR

#include "LudumGame.h"
#include "CalfAgent.h"
#include "FlockingDataCache.h"
#include "TP_TopDown/TP_TopDownCharacter.h"

ACalfAgent::ACalfAgent()
{
	m_currentSize = 0;
}


void ACalfAgent::SetCalfSize(int32 sizeIndex)
{
	if (!m_sizes.IsValidIndex(sizeIndex))
	{
		m_currentSize = 0;
		return;
	}

	m_currentSize = sizeIndex;
	m_health = m_sizes[m_currentSize].m_health;

	m_capsuleComponent->SetCapsuleHalfHeight(m_sizes[m_currentSize].m_capsuleHalfHeight);
	m_capsuleComponent->SetCapsuleRadius(m_sizes[m_currentSize].m_capsuleRadius);

	m_spriteComponent->SetRelativeScale3D(FVector(m_sizes[m_currentSize].m_scale));
}

void ACalfAgent::BeginPlay()
{
	Super::BeginPlay();

	SetCalfSize(m_currentSize);
}

void ACalfAgent::KillAgent()
{
	int32 numCalfsIncludingThis = UFlockingDataCache::GetNumAgentsInTeam(this, TEAM_CALVES);

	Super::KillAgent();

	ATP_TopDownCharacter *playerPawn = Cast<ATP_TopDownCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn());
	if (playerPawn)
	{
		if (numCalfsIncludingThis <= 1)
		{
			playerPawn->PlayVOEvent(ESystematicVOEvent::ChildrenAllDead);
		}
		else
		{			
			if (m_currentSize == 0)
			{
				playerPawn->PlayVOEvent(ESystematicVOEvent::ChildSmallDead);
			}
			else if (m_currentSize == 1)
			{
				playerPawn->PlayVOEvent(ESystematicVOEvent::ChildMiddleDead);
			}
			else
			{
				playerPawn->PlayVOEvent(ESystematicVOEvent::ChildBigDead);
			}
		}
	}
}

void ACalfAgent::ModifyHealth(float modificationAmount)
{
	Super::ModifyHealth(modificationAmount);

	ATP_TopDownCharacter *playerPawn = Cast<ATP_TopDownCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn());
	if (playerPawn)
	{
		if (m_health > 0 && modificationAmount < 0.0f)
		{
			int32 numCalfsIncludingThis = UFlockingDataCache::GetNumAgentsInTeam(this, TEAM_CALVES);

			if (numCalfsIncludingThis <= 1)
			{
				playerPawn->PlayVOEvent(ESystematicVOEvent::ChildTakesDamage);
			}
			else
			{
				playerPawn->PlayVOEvent(ESystematicVOEvent::ChildrenTakeDamage);
			}
		}
	}
}

void ACalfAgent::ChangeTeam(int32 newTeam)
{
	int32 numCalfsNotIncludingThis = UFlockingDataCache::GetNumAgentsInTeam(this, TEAM_CALVES);
	
	ATP_TopDownCharacter *playerPawn = Cast<ATP_TopDownCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn());
	if (playerPawn)
	{
		if (numCalfsNotIncludingThis == 0)
		{
			playerPawn->PlayVOEvent(ESystematicVOEvent::ChildFollowing);
		}
		else
		{
			playerPawn->PlayVOEvent(ESystematicVOEvent::ChildrenFollowing);
		}
	}

	Super::ChangeTeam(newTeam);
}