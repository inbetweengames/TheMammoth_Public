// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
// copyright 2015 inbetweengames GBR

#include "LudumGame.h"
#include "TP_TopDownGameMode.h"
#include "TP_TopDownPlayerController.h"
#include "TP_TopDownCharacter.h"

ATP_TopDownGameMode::ATP_TopDownGameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = ATP_TopDownPlayerController::StaticClass();
}