// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
// copyright 2015 inbetweengames GBR

#include "LudumGame.h"
#include "FlockingGameMode.h"

#include "Private/FlockingDataCache.h"

AFlockingGameMode::AFlockingGameMode()
{
	m_dataCache = CreateDefaultSubobject<UFlockingDataCache>(TEXT("FlockingDataCache"));
}
