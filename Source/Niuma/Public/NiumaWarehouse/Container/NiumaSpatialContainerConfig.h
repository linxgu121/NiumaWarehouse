#pragma once

#include "CoreMinimal.h"
#include "Misc/Guid.h"

#include "NiumaWarehouse/Type/NiumaItemContainerType.h"
#include "NiumaWarehouse/Type/NiumaItemType.h"

#include "NiumaSpatialContainerConfig.generated.h"

/**
 * 通用二维空间容器的运行时配置。
 * 只保存容器规则，不保存物品放置状态和占用缓存。
 */
USTRUCT(BlueprintType)
struct NIUMA_API FNiumaSpatialContainerConfig
{
	GENERATED_BODY()

public:
    /**
    * 创建一个新的空间容器配置。
    * 成功时生成新的 ContainerId；
    * 失败时不修改 OutConfig。
    */
    static bool TryCreate(
        ENiumaItemContainerType InContainerType,
        int32 InWidth,
        int32 InHeight,
        const TSet<ENiumaItemType>& InAcceptedItemTypes,
        FNiumaSpatialContainerConfig& OutConfig,
        FString* OutError = nullptr);

    /**
     * 检查运行时配置自身是否合法。
     */
    bool IsValid(FString* OutError = nullptr) const;

    /**
     * 当前容器实例的唯一标识。
     */
    UPROPERTY(BlueprintReadOnly, Category = "Niuma|Warehouse|ContainerConfig")
    FGuid ContainerId;

    /**
     * 容器的业务用途。
     */
    UPROPERTY(BlueprintReadOnly,Category = "Niuma|Warehouse|ContainerConfig")
    ENiumaItemContainerType ContainerType = ENiumaItemContainerType::None;

    /**
     * 二维容器横向逻辑格数量。
     */
    UPROPERTY(BlueprintReadOnly, Category = "Niuma|Warehouse|ContainerConfig")
    int32 Width = 0;

    /**
     * 二维容器纵向逻辑格数量。
     */
    UPROPERTY(BlueprintReadOnly, Category = "Niuma|Warehouse|ContainerConfig")
    int32 Height = 0;

    /**
    * 当前容器允许接收的物品类型。
    */
    UPROPERTY(BlueprintReadOnly, Category = "Niuma|Warehouse|ContainerConfig")
    TSet<ENiumaItemType> AcceptedItemTypes;
};
