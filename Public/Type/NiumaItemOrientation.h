#pragma once

#include "CoreMinimal.h"

#include "NiumaItemOrientation.generated.h"



UENUM(BlueprintType)
/**
 * 物品当前在仓库平面中的旋转方向。
 */
enum class ENiumaItemOrientation : uint8
{
	Degree0 = 0 UMETA(DisplayName = "0度"),
	Degree90 = 1 UMETA(DisplayName = "90度"),
	Degree180 = 2 UMETA(DisplayName = "180度"),
	Degree270 = 3 UMETA(DisplayName = "270度")
};