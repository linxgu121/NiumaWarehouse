#pragma once

#include "CoreMinimal.h"

#include "NiumaWarehouse/Item/NiumaItemInstance.h"
#include "NiumaWarehouse/Type/NiumaItemOrientation.h"

#include "NiumaSpatialItemPlacement.generated.h"


/**
 * 一份物品实例在二维空间容器中的放置记录。
 */
USTRUCT(BlueprintType)
struct NIUMA_API FNiumaSpatialItemPlacement
{
	GENERATED_BODY()

public:

    /**
     * 创建一条新的二维放置记录。
     * 失败时不修改 OutPlacement。
     */
    static bool TryCreate(
        const FNiumaItemInstance& InItem,
        FIntPoint InOrigin,
        ENiumaItemOrientation InOrientation,
        FNiumaSpatialItemPlacement& OutPlacement,
        FString* OutError = nullptr);

    /**
     * 检查放置记录自身的结构是否合法。
     * 不负责检查仓库边界与格子碰撞。
     */
    bool IsValid(
        FString* OutError = nullptr) const;

    /**
     * 被放置的物品实例。
     */
    UPROPERTY(BlueprintReadOnly,Category = "Niuma|Warehouse|Placement")
    FNiumaItemInstance Item;

    /**
     * 物品规范化 Footprint 的仓库二维原点。
     */
    UPROPERTY(BlueprintReadOnly, Category = "Niuma|Warehouse|Placement")
    FIntPoint Origin = FIntPoint::ZeroValue;

    /**
     * 物品相对于原始 Footprint 的当前方向。
     */
    UPROPERTY(BlueprintReadOnly, Category = "Niuma|Warehouse|Placement")
    ENiumaItemOrientation Orientation = ENiumaItemOrientation::Degree0;
};