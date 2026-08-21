#pragma once

#include "CoreMinimal.h"

#include "NiumaWarehouse/Spatial/NiumaSpatialItemPlacement.h"

#include "NiumaSpatialContainerState.generated.h"

USTRUCT(BlueprintType)
struct NIUMA_API FNiumaSpatialContainerState 
{
	GENERATED_BODY()

public:
	/**
     * 检查 State 自身的结构是否合法。
     * 不负责仓库边界、空间碰撞和物品定义解析。
     */
	bool IsStructurallyValid(FString* OutError = nullptr) const;

	/**
	 * 当前容器中的全部物品放置记录。
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Niuma|Warehouse|ContainerState")
	TArray<FNiumaSpatialItemPlacement> Placements;

	/**
	 * 容器状态版本。
	 * 每次成功修改容器时只增加一次。
	 */
	UPROPERTY(BlueprintReadOnly,Category = "Niuma|Warehouse|ContainerState")
	int64 Revision = 0;
};