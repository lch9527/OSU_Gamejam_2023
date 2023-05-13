// Copyright Epic Games, Inc. All Rights Reserved.

#include "OSU_Gamejam_2023GameMode.h"
#include "OSU_Gamejam_2023Character.h"
#include "UObject/ConstructorHelpers.h"

AOSU_Gamejam_2023GameMode::AOSU_Gamejam_2023GameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
