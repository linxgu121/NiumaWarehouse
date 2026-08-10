#pragma once

#include "CoreMinimal.h"

#include "NiumaItemContainerType.generated.h"

/**
 * 空间容器的业务用途
 * 类型只描述容器是什么，不决定其所有者和生命周期
 */
UENUM(BlueprintType)
enum class ENiumaItemContainerType : uint8 
{
	None = 0 UMETA(DisplayName = "未定义"),
	Warehouse = 10 UMETA(DisplayName = "仓库"),
	Backpack = 20 UMETA(DisplayName = "背包"),
	Corpse = 30 UMETA(DisplayName = "尸体")
};