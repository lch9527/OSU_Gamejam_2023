﻿#pragma once

#include "UObject/ObjectMacros.h"

UENUM(BlueprintType)
enum ECustomMovementMode
{
	CMOVE_Climbing      UMETA(DisplayName = "Climbing"),
	CMOVE_Hooking       UMETA(DisplayName = "Hooking"),
	CMOVE_WallRun       UMETA(DisplayName = "WallRun"),
	CMOVE_MAX			UMETA(Hidden),
};