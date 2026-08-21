#pragma once

#include "CoreMinimal.h"

#include "NiumaAccountSessionState.generated.h"

UENUM(BlueprintType)
enum class ENiumaAccountSessionState : uint8
{
	LoggedOut UMETA(DisplayName = "未登录"),
	LoggingIn UMETA(DisplayName = "登录中"),
	Authenticated UMETA(DisplayName = "已登录")
};