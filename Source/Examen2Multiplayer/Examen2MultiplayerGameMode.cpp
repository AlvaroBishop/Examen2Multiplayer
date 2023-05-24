// Copyright Epic Games, Inc. All Rights Reserved.

#include "Examen2MultiplayerGameMode.h"
#include "Examen2MultiplayerCharacter.h"
#include "UObject/ConstructorHelpers.h"

AExamen2MultiplayerGameMode::AExamen2MultiplayerGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
