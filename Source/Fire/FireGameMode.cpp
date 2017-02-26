// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "Fire.h"
#include "FireGameMode.h"
#include "FireCharacter.h"

AFireGameMode::AFireGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
