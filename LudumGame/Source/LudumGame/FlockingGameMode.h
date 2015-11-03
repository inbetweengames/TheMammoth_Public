// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
// copyright 2015 inbetweengames GBR

#pragma once
#include "GameFramework/GameMode.h"
#include "FlockingGameMode.generated.h"

UCLASS(minimalapi)
class AFlockingGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AFlockingGameMode();

	UPROPERTY()
	class UFlockingDataCache *m_dataCache;
};



