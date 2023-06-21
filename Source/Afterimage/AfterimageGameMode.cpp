// Copyright Epic Games, Inc. All Rights Reserved.

#include "AfterimageGameMode.h"
#include "AfterimageCharacter.h"
#include "UObject/ConstructorHelpers.h"

AAfterimageGameMode::AAfterimageGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
