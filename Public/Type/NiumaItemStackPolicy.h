#pragma once

#include "CoreMinimal.h"

#include "NiumaItemStackPolicy.generated.h"


/**
 * 物品的数量堆叠策略。
 * 这里只表示数量堆叠，不表示空间层叠。
 */
UENUM(BlueprintType)
enum class ENiumaItemStackPolicy : uint8
{
	NonStackable = 0 UMETA(DisplayName = "不可堆叠"),
	Stackable = 1 UMETA(DisplayName = "可堆叠")
};
