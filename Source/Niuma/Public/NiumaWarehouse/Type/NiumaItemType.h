#pragma once

#include "CoreMinimal.h"

#include "NiumaItemType.generated.h"

/**
 * 仓库系统当前支持的物品大类
 */
UENUM(BlueprintType)
enum class ENiumaItemType : uint8
{
	None = 0 UMETA(DisplayName = "未定义"),
	Weapon = 10 UMETA(DisplayName = "武器"),
	Armor = 20 UMETA(DisplayName = "防具"),
	StorageItem = 30 UMETA(DisplayName = "储物道具"),
	Consumable = 40 UMETA(DisplayName = "消耗品"),
	Miscellaneous = 50 UMETA(DisplayName = "杂物")
};
