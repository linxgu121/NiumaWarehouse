#pragma once

#include "CoreMinimal.h"

#include "NiumaItemRotationPolicy.generated.h"

/**
* 物品在仓库平面中的旋转策略。
* 这里只表示是否允许旋转，不保存物品当前方向。
*/
UENUM(BlueprintType)
enum class ENiumaItemRotationPolicy : uint8
{
	Fixed = 0 UMETA(DisplayName = "不允许旋转"),
	QuarterTurns = 1 UMETA(DisplayName = "允许单次90度旋转(可以旋转多次)")
};