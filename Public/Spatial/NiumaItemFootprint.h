#pragma once

#include "CoreMinimal.h"

#include "NiumaItemFootprint.generated.h"

/**
 * 物品在默认方向下占用的二维逻辑格
 * Cells 保存相对于物品原点的局部坐标
 */
USTRUCT(BlueprintType)
struct NIUMA_API FNiumaItemFootprint
{
	GENERATED_BODY()

public:
	FNiumaItemFootprint()
	{
		// 默认物品至少占用一个逻辑格。
		Cells.Add(FIntPoint::ZeroValue);
	}

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category = "Niuma|Warehouse|Spatial")
	TArray<FIntPoint> Cells;
};
